/**************************************************************************/
/*  steam_multiplayer_peer.cpp                                            */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             BLAZIUM ENGINE                             */
/*                          https://blazium.app                           */
/**************************************************************************/
/* Copyright (c) 2024-present Blazium Engine contributors.                */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#include "steam_multiplayer_peer.h"

#include "steam.h"

#include "core/io/marshalls.h"
#include "core/object/class_db.h"

// SteamSocketsTransport

SteamSocketsTransport::SteamSocketsTransport() {
	if (Steam::get_singleton()) {
		Steam::get_singleton()->register_networking_transport(this);
	}
}

SteamSocketsTransport::~SteamSocketsTransport() {
	close();
	if (Steam::get_singleton()) {
		Steam::get_singleton()->unregister_networking_transport(this);
	}
}

void SteamSocketsTransport::on_connection_status_changed(const SteamNetConnectionEvent &p_event) {
	if (peer) {
		peer->queue_connection_event(p_event);
	}
}

void SteamSocketsTransport::on_steam_shutdown() {
	if (peer) {
		peer->close();
	}
	poll_group = STEAM_NET_POLL_GROUP_INVALID;
}

Error SteamSocketsTransport::open() {
	Steam *steam = Steam::get_singleton();
	ERR_FAIL_NULL_V_MSG(steam, ERR_UNAVAILABLE, "The Steam singleton is not available.");
	ERR_FAIL_COND_V_MSG(!steam->is_initialized(), ERR_UNCONFIGURED, "Steam must be initialized with Steam.initialize() before creating a SteamMultiplayerPeer.");
	ERR_FAIL_COND_V_MSG(!steam->has_networking_support(), ERR_UNAVAILABLE, "The loaded Steam API library doesn't expose ISteamNetworkingSockets.");

	steam->init_relay_network_access();
	if (poll_group == STEAM_NET_POLL_GROUP_INVALID) {
		poll_group = steam->get_loader().net_create_poll_group(steam->get_networking_sockets_interface());
		ERR_FAIL_COND_V_MSG(poll_group == STEAM_NET_POLL_GROUP_INVALID, ERR_CANT_CREATE, "Failed to create a Steam Networking Sockets poll group.");
	}
	return OK;
}

void SteamSocketsTransport::close() {
	if (poll_group == STEAM_NET_POLL_GROUP_INVALID) {
		return;
	}
	Steam *steam = Steam::get_singleton();
	if (steam && steam->is_initialized()) {
		steam->get_loader().net_destroy_poll_group(steam->get_networking_sockets_interface(), poll_group);
	}
	poll_group = STEAM_NET_POLL_GROUP_INVALID;
}

SteamHSteamListenSocket SteamSocketsTransport::create_listen_socket(int p_virtual_port) {
	Steam *steam = Steam::get_singleton();
	ERR_FAIL_NULL_V(steam, STEAM_LISTEN_SOCKET_INVALID);
	return steam->get_loader().net_create_listen_socket_p2p(steam->get_networking_sockets_interface(), p_virtual_port);
}

void SteamSocketsTransport::close_listen_socket(SteamHSteamListenSocket p_socket) {
	Steam *steam = Steam::get_singleton();
	if (steam && steam->is_initialized()) {
		steam->get_loader().net_close_listen_socket(steam->get_networking_sockets_interface(), p_socket);
	}
}

SteamHSteamNetConnection SteamSocketsTransport::connect(uint64_t p_steam_id, int p_virtual_port) {
	Steam *steam = Steam::get_singleton();
	ERR_FAIL_NULL_V(steam, STEAM_NET_CONNECTION_INVALID);
	const SteamAPILoader &loader = steam->get_loader();
	SteamAPILoader::ISteamNetworkingSocketsPtr sockets = steam->get_networking_sockets_interface();
	SteamHSteamNetConnection connection = loader.net_connect_p2p(sockets, p_steam_id, p_virtual_port);
	if (connection != STEAM_NET_CONNECTION_INVALID) {
		loader.net_set_connection_poll_group(sockets, connection, poll_group);
	}
	return connection;
}

bool SteamSocketsTransport::accept_connection(SteamHSteamNetConnection p_connection) {
	Steam *steam = Steam::get_singleton();
	ERR_FAIL_NULL_V(steam, false);
	const SteamAPILoader &loader = steam->get_loader();
	SteamAPILoader::ISteamNetworkingSocketsPtr sockets = steam->get_networking_sockets_interface();
	if (loader.net_accept_connection(sockets, p_connection) != STEAM_RESULT_OK) {
		return false;
	}
	return loader.net_set_connection_poll_group(sockets, p_connection, poll_group);
}

void SteamSocketsTransport::close_connection(SteamHSteamNetConnection p_connection, int p_reason, const String &p_debug, bool p_linger) {
	Steam *steam = Steam::get_singleton();
	if (steam && steam->is_initialized()) {
		steam->get_loader().net_close_connection(steam->get_networking_sockets_interface(), p_connection, p_reason, p_debug.utf8().get_data(), p_linger);
	}
}

Error SteamSocketsTransport::send_message(SteamHSteamNetConnection p_connection, const uint8_t *p_data, int p_size, bool p_reliable) {
	Steam *steam = Steam::get_singleton();
	ERR_FAIL_NULL_V(steam, ERR_UNAVAILABLE);
	// Nagle is disabled: SceneMultiplayer doesn't batch messages per frame, so
	// waiting for more data would only add latency.
	const int flags = (p_reliable ? STEAM_NETWORKING_SEND_RELIABLE : STEAM_NETWORKING_SEND_UNRELIABLE) | STEAM_NETWORKING_SEND_NO_NAGLE;
	const int result = steam->get_loader().net_send_message_to_connection(steam->get_networking_sockets_interface(), p_connection, p_data, (uint32_t)p_size, flags);
	if (result == STEAM_RESULT_OK) {
		return OK;
	}
	return result == STEAM_RESULT_LIMIT_EXCEEDED ? ERR_BUSY : FAILED;
}

void SteamSocketsTransport::receive_messages(List<Message> &r_messages) {
	Steam *steam = Steam::get_singleton();
	if (!steam || !steam->is_initialized() || poll_group == STEAM_NET_POLL_GROUP_INVALID) {
		return;
	}
	const SteamAPILoader &loader = steam->get_loader();
	SteamAPILoader::ISteamNetworkingSocketsPtr sockets = steam->get_networking_sockets_interface();

	const int kBatchSize = 64;
	SteamNetworkingMessage *messages[kBatchSize];
	while (true) {
		const int count = loader.net_receive_messages_on_poll_group(sockets, poll_group, messages, kBatchSize);
		for (int i = 0; i < count; i++) {
			SteamNetworkingMessage *message = messages[i];
			if (message->m_cbSize > 0 && message->m_pData) {
				Message &received = r_messages.push_back(Message())->get();
				received.connection = message->m_conn;
				received.data.resize(message->m_cbSize);
				memcpy(received.data.ptrw(), message->m_pData, message->m_cbSize);
			}
			loader.net_release_message(message);
		}
		if (count < kBatchSize) {
			break;
		}
	}
}

int SteamSocketsTransport::get_ping(SteamHSteamNetConnection p_connection) const {
	Steam *steam = Steam::get_singleton();
	if (!steam || !steam->is_initialized()) {
		return -1;
	}
	SteamNetConnectionRealTimeStatus status;
	if (!steam->get_loader().net_get_connection_real_time_status(steam->get_networking_sockets_interface(), p_connection, status)) {
		return -1;
	}
	return status.m_nPing;
}

void SteamSocketsTransport::poll() {
	Steam *steam = Steam::get_singleton();
	if (steam && steam->is_initialized()) {
		steam->poll_callbacks();
	}
}

// SteamMultiplayerPeer

void SteamMultiplayerPeer::_bind_methods() {
	ClassDB::bind_method(D_METHOD("create_host", "virtual_port", "max_clients"), &SteamMultiplayerPeer::create_host, DEFVAL(0), DEFVAL(32));
	ClassDB::bind_method(D_METHOD("create_client", "host_steam_id", "virtual_port"), &SteamMultiplayerPeer::create_client, DEFVAL(0));
	ClassDB::bind_method(D_METHOD("get_peer_steam_id", "peer_id"), &SteamMultiplayerPeer::get_peer_steam_id);
	ClassDB::bind_method(D_METHOD("get_peer_id_for_steam_id", "steam_id"), &SteamMultiplayerPeer::get_peer_id_for_steam_id);
	ClassDB::bind_method(D_METHOD("get_peer_ping", "peer_id"), &SteamMultiplayerPeer::get_peer_ping);
	ClassDB::bind_method(D_METHOD("get_max_clients"), &SteamMultiplayerPeer::get_max_clients);
}

SteamMultiplayerPeer::SteamMultiplayerPeer() {
}

SteamMultiplayerPeer::~SteamMultiplayerPeer() {
	close();
	if (transport) {
		memdelete(transport);
		transport = nullptr;
	}
}

void SteamMultiplayerPeer::set_transport(SteamNetworkingTransport *p_transport) {
	ERR_FAIL_COND_MSG(mode != MODE_NONE, "Can't change the transport of an active SteamMultiplayerPeer.");
	if (transport) {
		memdelete(transport);
	}
	transport = p_transport;
	if (transport) {
		transport->set_peer(this);
	}
}

SteamNetworkingTransport *SteamMultiplayerPeer::_get_transport() {
	if (!transport) {
		set_transport(memnew(SteamSocketsTransport));
	}
	return transport;
}

void SteamMultiplayerPeer::queue_connection_event(const SteamNetConnectionEvent &p_event) {
	if (mode == MODE_NONE) {
		return;
	}
	pending_events.push_back(p_event);
}

Error SteamMultiplayerPeer::create_host(int p_virtual_port, int p_max_clients) {
	ERR_FAIL_COND_V_MSG(mode != MODE_NONE, ERR_ALREADY_IN_USE, "The multiplayer instance is already active.");
	ERR_FAIL_COND_V(p_virtual_port < 0, ERR_INVALID_PARAMETER);
	ERR_FAIL_COND_V(p_max_clients < 0, ERR_INVALID_PARAMETER);

	SteamNetworkingTransport *t = _get_transport();
	Error err = t->open();
	if (err != OK) {
		return err;
	}
	listen_socket = t->create_listen_socket(p_virtual_port);
	if (listen_socket == STEAM_LISTEN_SOCKET_INVALID) {
		t->close();
		ERR_FAIL_V_MSG(ERR_CANT_CREATE, vformat("Failed to create a Steam P2P listen socket on virtual port %d.", p_virtual_port));
	}

	mode = MODE_SERVER;
	max_clients = p_max_clients;
	unique_id = 1;
	connection_status = CONNECTION_CONNECTED;
	return OK;
}

Error SteamMultiplayerPeer::create_client(uint64_t p_host_steam_id, int p_virtual_port) {
	ERR_FAIL_COND_V_MSG(mode != MODE_NONE, ERR_ALREADY_IN_USE, "The multiplayer instance is already active.");
	ERR_FAIL_COND_V_MSG(p_host_steam_id == 0, ERR_INVALID_PARAMETER, "Invalid host Steam ID.");
	ERR_FAIL_COND_V(p_virtual_port < 0, ERR_INVALID_PARAMETER);

	SteamNetworkingTransport *t = _get_transport();
	Error err = t->open();
	if (err != OK) {
		return err;
	}
	server_connection = t->connect(p_host_steam_id, p_virtual_port);
	if (server_connection == STEAM_NET_CONNECTION_INVALID) {
		t->close();
		ERR_FAIL_V_MSG(ERR_CANT_CONNECT, vformat("Failed to start a Steam P2P connection to %d.", p_host_steam_id));
	}

	mode = MODE_CLIENT;
	server_steam_id = p_host_steam_id;
	unique_id = 0; // Assigned by the server once connected.
	connection_status = CONNECTION_CONNECTING;
	return OK;
}

void SteamMultiplayerPeer::_reset() {
	mode = MODE_NONE;
	connection_status = CONNECTION_DISCONNECTED;
	unique_id = 0;
	target_peer = 0;
	max_clients = 0;
	listen_socket = STEAM_LISTEN_SOCKET_INVALID;
	server_connection = STEAM_NET_CONNECTION_INVALID;
	server_steam_id = 0;
	peers.clear();
	connection_to_peer.clear();
	pending_connections.clear();
	pending_events.clear();
	incoming_packets.clear();
	current_packet = Packet();
	ordered_send_sequence.clear();
	ordered_receive_sequence.clear();
}

void SteamMultiplayerPeer::close() {
	if (mode == MODE_NONE) {
		return;
	}
	if (transport) {
		for (const KeyValue<SteamHSteamNetConnection, int> &E : connection_to_peer) {
			transport->close_connection(E.key, STEAM_NET_CONNECTION_END_APP_GENERIC, "Closed", true);
		}
		for (const KeyValue<SteamHSteamNetConnection, uint64_t> &E : pending_connections) {
			transport->close_connection(E.key, STEAM_NET_CONNECTION_END_APP_GENERIC, "Closed", false);
		}
		if (server_connection != STEAM_NET_CONNECTION_INVALID && !connection_to_peer.has(server_connection)) {
			transport->close_connection(server_connection, STEAM_NET_CONNECTION_END_APP_GENERIC, "Closed", false);
		}
		if (listen_socket != STEAM_LISTEN_SOCKET_INVALID) {
			transport->close_listen_socket(listen_socket);
		}
		transport->close();
	}
	_reset();
}

void SteamMultiplayerPeer::_drop_connection(SteamHSteamNetConnection p_connection) {
	pending_connections.erase(p_connection);
	HashMap<SteamHSteamNetConnection, int>::Iterator E = connection_to_peer.find(p_connection);
	if (!E) {
		return;
	}
	const int peer_id = E->value;
	connection_to_peer.remove(E);
	peers.erase(peer_id);
	for (HashMap<uint64_t, uint32_t>::Iterator S = ordered_receive_sequence.begin(); S;) {
		HashMap<uint64_t, uint32_t>::Iterator next = S;
		++next;
		if ((S->key >> 32) == p_connection) {
			ordered_receive_sequence.remove(S);
		}
		S = next;
	}
	emit_signal(SNAME("peer_disconnected"), peer_id);
}

void SteamMultiplayerPeer::_process_event(const SteamNetConnectionEvent &p_event) {
	if (mode == MODE_SERVER) {
		_process_server_event(p_event);
	} else if (mode == MODE_CLIENT) {
		_process_client_event(p_event);
	}
}

void SteamMultiplayerPeer::_process_server_event(const SteamNetConnectionEvent &p_event) {
	const SteamHSteamNetConnection connection = p_event.connection;
	const bool known = connection_to_peer.has(connection) || pending_connections.has(connection);
	if (!known && (p_event.listen_socket != listen_socket || listen_socket == STEAM_LISTEN_SOCKET_INVALID)) {
		return; // Not ours (another peer or an outgoing connection).
	}

	switch (p_event.state) {
		case STEAM_NET_CONNECTION_STATE_CONNECTING: {
			if (known) {
				return;
			}
			const bool full = max_clients > 0 && (int)(peers.size() + pending_connections.size()) >= max_clients;
			if (is_refusing_new_connections() || full) {
				transport->close_connection(connection, STEAM_NET_CONNECTION_END_APP_SERVER_FULL, full ? "Server full" : "Not accepting connections", false);
				return;
			}
			if (!transport->accept_connection(connection)) {
				transport->close_connection(connection, STEAM_NET_CONNECTION_END_APP_GENERIC, "Accept failed", false);
				return;
			}
			pending_connections[connection] = p_event.remote_steam_id;
		} break;
		case STEAM_NET_CONNECTION_STATE_CONNECTED: {
			HashMap<SteamHSteamNetConnection, uint64_t>::Iterator E = pending_connections.find(connection);
			if (!E) {
				return;
			}
			const uint64_t steam_id = E->value != 0 ? E->value : p_event.remote_steam_id;
			pending_connections.remove(E);

			int peer_id = generate_unique_id();
			while (peer_id == 1 || peers.has(peer_id)) {
				peer_id = generate_unique_id();
			}

			uint8_t assign[ASSIGN_ID_SIZE];
			assign[0] = MESSAGE_ASSIGN_ID;
			assign[1] = PROTOCOL_VERSION;
			encode_uint32((uint32_t)peer_id, &assign[2]);
			if (transport->send_message(connection, assign, ASSIGN_ID_SIZE, true) != OK) {
				transport->close_connection(connection, STEAM_NET_CONNECTION_END_APP_GENERIC, "Handshake failed", false);
				return;
			}

			RemotePeer remote;
			remote.connection = connection;
			remote.steam_id = steam_id;
			peers[peer_id] = remote;
			connection_to_peer[connection] = peer_id;
			emit_signal(SNAME("peer_connected"), peer_id);
		} break;
		case STEAM_NET_CONNECTION_STATE_CLOSED_BY_PEER:
		case STEAM_NET_CONNECTION_STATE_PROBLEM_DETECTED_LOCALLY: {
			// The handle must still be closed to free it.
			transport->close_connection(connection, STEAM_NET_CONNECTION_END_APP_GENERIC, String(), false);
			_drop_connection(connection);
		} break;
		default:
			break;
	}
}

void SteamMultiplayerPeer::_process_client_event(const SteamNetConnectionEvent &p_event) {
	if (p_event.connection != server_connection || server_connection == STEAM_NET_CONNECTION_INVALID) {
		return;
	}
	if (p_event.state == STEAM_NET_CONNECTION_STATE_CLOSED_BY_PEER || p_event.state == STEAM_NET_CONNECTION_STATE_PROBLEM_DETECTED_LOCALLY) {
		const bool was_connected = connection_status == CONNECTION_CONNECTED;
		transport->close_connection(server_connection, STEAM_NET_CONNECTION_END_APP_GENERIC, String(), false);
		server_connection = STEAM_NET_CONNECTION_INVALID;
		connection_to_peer.clear();
		peers.clear();
		if (was_connected) {
			emit_signal(SNAME("peer_disconnected"), 1);
		}
		close();
	}
	// CONNECTED: wait for MESSAGE_ASSIGN_ID before reporting the connection.
}

void SteamMultiplayerPeer::_process_message(const SteamNetworkingTransport::Message &p_message) {
	const int size = p_message.data.size();
	if (size < 1) {
		return;
	}
	const uint8_t *data = p_message.data.ptr();

	int from = 0;
	if (mode == MODE_SERVER) {
		HashMap<SteamHSteamNetConnection, int>::Iterator E = connection_to_peer.find(p_message.connection);
		if (!E) {
			return;
		}
		from = E->value;
	} else {
		if (p_message.connection != server_connection) {
			return;
		}
		if (data[0] == MESSAGE_ASSIGN_ID) {
			if (connection_status != CONNECTION_CONNECTING || size < ASSIGN_ID_SIZE) {
				return;
			}
			if (data[1] != PROTOCOL_VERSION) {
				ERR_PRINT(vformat("SteamMultiplayerPeer protocol mismatch (local %d, host %d); disconnecting.", PROTOCOL_VERSION, data[1]));
				close();
				return;
			}
			const int id = (int)decode_uint32(&data[2]);
			if (id <= 1) {
				close();
				return;
			}
			unique_id = id;
			RemotePeer server;
			server.connection = server_connection;
			server.steam_id = server_steam_id;
			peers[1] = server;
			connection_to_peer[server_connection] = 1;
			connection_status = CONNECTION_CONNECTED;
			emit_signal(SNAME("peer_connected"), 1);
			return;
		}
		if (connection_status != CONNECTION_CONNECTED) {
			return; // Data before the handshake (unreliable messages can overtake it).
		}
		from = 1;
	}

	if (data[0] != MESSAGE_DATA || size < DATA_HEADER_SIZE) {
		return;
	}
	const int packet_mode = data[1];
	if (packet_mode > TRANSFER_MODE_RELIABLE) {
		return;
	}
	const int channel = data[2];
	int offset = DATA_HEADER_SIZE;
	if (packet_mode == TRANSFER_MODE_UNRELIABLE_ORDERED) {
		if (size < DATA_HEADER_SIZE + SEQUENCE_SIZE) {
			return;
		}
		const uint32_t sequence = decode_uint32(&data[DATA_HEADER_SIZE]);
		const uint64_t key = ((uint64_t)p_message.connection << 32) | (uint32_t)channel;
		HashMap<uint64_t, uint32_t>::Iterator S = ordered_receive_sequence.find(key);
		if (S && (int32_t)(sequence - S->value) <= 0) {
			return; // Older than (or same as) the last delivered packet.
		}
		ordered_receive_sequence[key] = sequence;
		offset += SEQUENCE_SIZE;
	}

	Packet packet;
	packet.from = from;
	packet.channel = channel;
	packet.transfer_mode = (TransferMode)packet_mode;
	packet.data.resize(size - offset);
	if (size > offset) {
		memcpy(packet.data.ptrw(), data + offset, size - offset);
	}
	incoming_packets.push_back(packet);
}

void SteamMultiplayerPeer::poll() {
	if (mode == MODE_NONE || !transport) {
		return;
	}
	transport->poll();

	while (mode != MODE_NONE && !pending_events.is_empty()) {
		const SteamNetConnectionEvent event = pending_events.front()->get();
		pending_events.pop_front();
		_process_event(event);
	}
	if (mode == MODE_NONE) {
		return;
	}

	List<SteamNetworkingTransport::Message> messages;
	transport->receive_messages(messages);
	for (const SteamNetworkingTransport::Message &message : messages) {
		if (mode == MODE_NONE) {
			break;
		}
		_process_message(message);
	}
}

Error SteamMultiplayerPeer::_send_to(SteamHSteamNetConnection p_connection, const uint8_t *p_data, int p_size, bool p_reliable) {
	return transport->send_message(p_connection, p_data, p_size, p_reliable);
}

Error SteamMultiplayerPeer::put_packet(const uint8_t *p_buffer, int p_buffer_size) {
	ERR_FAIL_COND_V_MSG(connection_status != CONNECTION_CONNECTED, ERR_UNCONFIGURED, "The multiplayer instance isn't currently connected.");
	ERR_FAIL_COND_V_MSG(p_buffer_size > get_max_packet_size(), ERR_OUT_OF_MEMORY, vformat("Packet of %d bytes exceeds the Steam message size limit (%d bytes).", p_buffer_size, get_max_packet_size()));
	ERR_FAIL_COND_V(p_buffer_size < 0, ERR_INVALID_PARAMETER);

	const TransferMode send_mode = get_transfer_mode();
	const int channel = get_transfer_channel();
	ERR_FAIL_COND_V_MSG(channel < 0 || channel > 255, ERR_INVALID_PARAMETER, "SteamMultiplayerPeer supports transfer channels 0 to 255.");
	const bool reliable = send_mode == TRANSFER_MODE_RELIABLE;

	const int header_size = DATA_HEADER_SIZE + (send_mode == TRANSFER_MODE_UNRELIABLE_ORDERED ? SEQUENCE_SIZE : 0);
	Vector<uint8_t> message;
	message.resize(header_size + p_buffer_size);
	uint8_t *w = message.ptrw();
	w[0] = MESSAGE_DATA;
	w[1] = (uint8_t)send_mode;
	w[2] = (uint8_t)channel;
	if (send_mode == TRANSFER_MODE_UNRELIABLE_ORDERED) {
		uint32_t &sequence = ordered_send_sequence[channel];
		sequence++;
		encode_uint32(sequence, &w[DATA_HEADER_SIZE]);
	}
	if (p_buffer_size > 0) {
		memcpy(w + header_size, p_buffer, p_buffer_size);
	}

	if (mode == MODE_CLIENT) {
		// Clients only talk to the server; SceneMultiplayer relays the rest.
		return _send_to(server_connection, message.ptr(), message.size(), reliable);
	}

	if (target_peer > 0) {
		HashMap<int, RemotePeer>::ConstIterator E = peers.find(target_peer);
		ERR_FAIL_COND_V_MSG(!E, ERR_INVALID_PARAMETER, vformat("Invalid target peer: %d", target_peer));
		return _send_to(E->value.connection, message.ptr(), message.size(), reliable);
	}

	Error result = OK;
	for (const KeyValue<int, RemotePeer> &E : peers) {
		if (target_peer < 0 && E.key == -target_peer) {
			continue;
		}
		Error err = _send_to(E.value.connection, message.ptr(), message.size(), reliable);
		if (err != OK) {
			result = err;
		}
	}
	return result;
}

int SteamMultiplayerPeer::get_available_packet_count() const {
	return incoming_packets.size();
}

Error SteamMultiplayerPeer::get_packet(const uint8_t **r_buffer, int &r_buffer_size) {
	ERR_FAIL_COND_V_MSG(incoming_packets.is_empty(), ERR_UNAVAILABLE, "No incoming packets available.");
	current_packet = incoming_packets.front()->get();
	incoming_packets.pop_front();
	*r_buffer = current_packet.data.ptr();
	r_buffer_size = current_packet.data.size();
	return OK;
}

int SteamMultiplayerPeer::get_max_packet_size() const {
	return STEAM_NETWORKING_MAX_MESSAGE_SIZE - DATA_HEADER_SIZE - SEQUENCE_SIZE;
}

void SteamMultiplayerPeer::set_target_peer(int p_peer_id) {
	target_peer = p_peer_id;
}

int SteamMultiplayerPeer::get_packet_peer() const {
	ERR_FAIL_COND_V(incoming_packets.is_empty(), 0);
	return incoming_packets.front()->get().from;
}

MultiplayerPeer::TransferMode SteamMultiplayerPeer::get_packet_mode() const {
	ERR_FAIL_COND_V(incoming_packets.is_empty(), TRANSFER_MODE_RELIABLE);
	return incoming_packets.front()->get().transfer_mode;
}

int SteamMultiplayerPeer::get_packet_channel() const {
	ERR_FAIL_COND_V(incoming_packets.is_empty(), 0);
	return incoming_packets.front()->get().channel;
}

void SteamMultiplayerPeer::disconnect_peer(int p_peer, bool p_force) {
	ERR_FAIL_COND(mode == MODE_NONE);
	if (mode == MODE_CLIENT) {
		ERR_FAIL_COND_MSG(p_peer != 1, "Clients can only disconnect from the server (peer 1).");
		const bool was_connected = connection_status == CONNECTION_CONNECTED;
		if (transport && server_connection != STEAM_NET_CONNECTION_INVALID) {
			transport->close_connection(server_connection, STEAM_NET_CONNECTION_END_APP_GENERIC, "Disconnected", !p_force);
		}
		connection_to_peer.clear();
		peers.clear();
		server_connection = STEAM_NET_CONNECTION_INVALID;
		if (was_connected) {
			emit_signal(SNAME("peer_disconnected"), 1);
		}
		close();
		return;
	}

	HashMap<int, RemotePeer>::Iterator E = peers.find(p_peer);
	ERR_FAIL_COND_MSG(!E, vformat("Invalid peer: %d", p_peer));
	const SteamHSteamNetConnection connection = E->value.connection;
	transport->close_connection(connection, STEAM_NET_CONNECTION_END_APP_GENERIC, "Disconnected", !p_force);
	_drop_connection(connection);
}

bool SteamMultiplayerPeer::is_server() const {
	return mode == MODE_SERVER;
}

int SteamMultiplayerPeer::get_unique_id() const {
	return unique_id;
}

MultiplayerPeer::ConnectionStatus SteamMultiplayerPeer::get_connection_status() const {
	return connection_status;
}

uint64_t SteamMultiplayerPeer::get_peer_steam_id(int p_peer_id) const {
	HashMap<int, RemotePeer>::ConstIterator E = peers.find(p_peer_id);
	if (!E) {
		return 0;
	}
	return E->value.steam_id;
}

int SteamMultiplayerPeer::get_peer_id_for_steam_id(uint64_t p_steam_id) const {
	if (p_steam_id == 0) {
		return 0;
	}
	for (const KeyValue<int, RemotePeer> &E : peers) {
		if (E.value.steam_id == p_steam_id) {
			return E.key;
		}
	}
	return 0;
}

int SteamMultiplayerPeer::get_peer_ping(int p_peer_id) const {
	HashMap<int, RemotePeer>::ConstIterator E = peers.find(p_peer_id);
	if (!E || !transport) {
		return -1;
	}
	return transport->get_ping(E->value.connection);
}
