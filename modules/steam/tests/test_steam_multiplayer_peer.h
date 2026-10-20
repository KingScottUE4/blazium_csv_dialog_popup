/**************************************************************************/
/*  test_steam_multiplayer_peer.h                                         */
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

#pragma once

#include "../steam.h"
#include "../steam_multiplayer_peer.h"

#include "core/templates/hash_map.h"
#include "core/templates/list.h"
#include "tests/signal_watcher.h"
#include "tests/test_macros.h"

#include "modules/modules_enabled.gen.h" // For multiplayer.

#ifdef MODULE_MULTIPLAYER_ENABLED
#include "modules/multiplayer/scene_multiplayer.h"
#endif

namespace TestSteamMultiplayerPeer {

// An in-memory stand-in for Steam Networking Sockets: connections between
// FakeSteamTransport instances identified by fake Steam IDs. Connection
// events are queued and only delivered when the owning peer polls, like the
// real callbacks.
class FakeSteamNetwork {
public:
	struct Connection {
		uint64_t owner = 0;
		uint64_t remote_steam_id = 0;
		SteamHSteamNetConnection other = STEAM_NET_CONNECTION_INVALID;
		SteamHSteamListenSocket listen_socket = STEAM_LISTEN_SOCKET_INVALID;
		int state = STEAM_NET_CONNECTION_STATE_NONE;
		List<Vector<uint8_t>> inbox;
	};

	struct QueuedEvent {
		uint64_t owner = 0;
		SteamNetConnectionEvent event;
	};

	HashMap<SteamHSteamNetConnection, Connection> connections;
	HashMap<uint64_t, HashMap<int, SteamHSteamListenSocket>> listeners;
	List<QueuedEvent> events;
	uint32_t next_handle = 100;
	int messages_sent = 0;

	void queue_event(SteamHSteamNetConnection p_connection, int p_old_state) {
		const Connection &c = connections[p_connection];
		QueuedEvent queued;
		queued.owner = c.owner;
		queued.event.connection = p_connection;
		queued.event.listen_socket = c.listen_socket;
		queued.event.remote_steam_id = c.remote_steam_id;
		queued.event.old_state = p_old_state;
		queued.event.state = c.state;
		events.push_back(queued);
	}

	void set_state(SteamHSteamNetConnection p_connection, int p_state) {
		Connection &c = connections[p_connection];
		const int old_state = c.state;
		c.state = p_state;
		queue_event(p_connection, old_state);
	}

	// Reverses the queued messages of a connection, to simulate reordering.
	void reverse_inbox(SteamHSteamNetConnection p_connection) {
		List<Vector<uint8_t>> &inbox = connections[p_connection].inbox;
		List<Vector<uint8_t>> reversed;
		for (const Vector<uint8_t> &message : inbox) {
			reversed.push_front(message);
		}
		inbox = reversed;
	}

	SteamHSteamNetConnection find_connection(uint64_t p_owner) const {
		for (const KeyValue<SteamHSteamNetConnection, Connection> &E : connections) {
			if (E.value.owner == p_owner) {
				return E.key;
			}
		}
		return STEAM_NET_CONNECTION_INVALID;
	}
};

class FakeSteamTransport : public SteamNetworkingTransport {
	FakeSteamNetwork *network = nullptr;
	uint64_t steam_id = 0;

public:
	virtual Error open() override { return OK; }
	virtual void close() override {}

	virtual SteamHSteamListenSocket create_listen_socket(int p_virtual_port) override {
		const SteamHSteamListenSocket socket = network->next_handle++;
		network->listeners[steam_id][p_virtual_port] = socket;
		return socket;
	}

	virtual void close_listen_socket(SteamHSteamListenSocket p_socket) override {
		HashMap<int, SteamHSteamListenSocket> &sockets = network->listeners[steam_id];
		for (const KeyValue<int, SteamHSteamListenSocket> &E : sockets) {
			if (E.value == p_socket) {
				sockets.erase(E.key);
				break;
			}
		}
	}

	virtual SteamHSteamNetConnection connect(uint64_t p_steam_id, int p_virtual_port) override {
		const SteamHSteamNetConnection local = network->next_handle++;
		FakeSteamNetwork::Connection &c = network->connections[local];
		c.owner = steam_id;
		c.remote_steam_id = p_steam_id;
		c.state = STEAM_NET_CONNECTION_STATE_CONNECTING;

		if (!network->listeners.has(p_steam_id) || !network->listeners[p_steam_id].has(p_virtual_port)) {
			network->set_state(local, STEAM_NET_CONNECTION_STATE_PROBLEM_DETECTED_LOCALLY);
			return local;
		}
		const SteamHSteamNetConnection remote = network->next_handle++;
		FakeSteamNetwork::Connection &r = network->connections[remote];
		r.owner = p_steam_id;
		r.remote_steam_id = steam_id;
		r.listen_socket = network->listeners[p_steam_id][p_virtual_port];
		r.other = local;
		network->connections[local].other = remote;
		network->set_state(remote, STEAM_NET_CONNECTION_STATE_CONNECTING);
		return local;
	}

	virtual bool accept_connection(SteamHSteamNetConnection p_connection) override {
		if (!network->connections.has(p_connection)) {
			return false;
		}
		const SteamHSteamNetConnection other = network->connections[p_connection].other;
		network->set_state(p_connection, STEAM_NET_CONNECTION_STATE_CONNECTED);
		if (network->connections.has(other)) {
			network->set_state(other, STEAM_NET_CONNECTION_STATE_CONNECTED);
		}
		return true;
	}

	virtual void close_connection(SteamHSteamNetConnection p_connection, int p_reason, const String &p_debug, bool p_linger) override {
		if (!network->connections.has(p_connection)) {
			return;
		}
		const SteamHSteamNetConnection other = network->connections[p_connection].other;
		network->connections.erase(p_connection);
		if (network->connections.has(other)) {
			FakeSteamNetwork::Connection &o = network->connections[other];
			o.other = STEAM_NET_CONNECTION_INVALID;
			if (o.state == STEAM_NET_CONNECTION_STATE_CONNECTING || o.state == STEAM_NET_CONNECTION_STATE_CONNECTED) {
				network->set_state(other, STEAM_NET_CONNECTION_STATE_CLOSED_BY_PEER);
			}
		}
	}

	virtual Error send_message(SteamHSteamNetConnection p_connection, const uint8_t *p_data, int p_size, bool p_reliable) override {
		if (!network->connections.has(p_connection)) {
			return FAILED;
		}
		const FakeSteamNetwork::Connection &c = network->connections[p_connection];
		if (c.state != STEAM_NET_CONNECTION_STATE_CONNECTED || !network->connections.has(c.other)) {
			return FAILED;
		}
		Vector<uint8_t> data;
		data.resize(p_size);
		memcpy(data.ptrw(), p_data, p_size);
		network->connections[c.other].inbox.push_back(data);
		network->messages_sent++;
		return OK;
	}

	virtual void receive_messages(List<Message> &r_messages) override {
		for (KeyValue<SteamHSteamNetConnection, FakeSteamNetwork::Connection> &E : network->connections) {
			if (E.value.owner != steam_id) {
				continue;
			}
			while (!E.value.inbox.is_empty()) {
				Message message;
				message.connection = E.key;
				message.data = E.value.inbox.front()->get();
				E.value.inbox.pop_front();
				r_messages.push_back(message);
			}
		}
	}

	virtual int get_ping(SteamHSteamNetConnection p_connection) const override { return 42; }

	virtual void poll() override {
		List<FakeSteamNetwork::QueuedEvent>::Element *E = network->events.front();
		while (E) {
			List<FakeSteamNetwork::QueuedEvent>::Element *next = E->next();
			if (E->get().owner == steam_id) {
				peer->queue_connection_event(E->get().event);
				network->events.erase(E);
			}
			E = next;
		}
	}

	FakeSteamTransport(FakeSteamNetwork *p_network, uint64_t p_steam_id) {
		network = p_network;
		steam_id = p_steam_id;
	}
};

static const uint64_t HOST_ID = 76561198000000001ULL;
static const uint64_t CLIENT_A_ID = 76561198000000002ULL;
static const uint64_t CLIENT_B_ID = 76561198000000003ULL;

static Ref<SteamMultiplayerPeer> make_peer(FakeSteamNetwork &p_network, uint64_t p_steam_id) {
	Ref<SteamMultiplayerPeer> peer;
	peer.instantiate();
	peer->set_transport(memnew(FakeSteamTransport(&p_network, p_steam_id)));
	return peer;
}

static void poll_all(const Vector<Ref<SteamMultiplayerPeer>> &p_peers, int p_rounds = 4) {
	for (int i = 0; i < p_rounds; i++) {
		for (const Ref<SteamMultiplayerPeer> &peer : p_peers) {
			peer->poll();
		}
	}
}

static Vector<uint8_t> read_packet(const Ref<SteamMultiplayerPeer> &p_peer) {
	const uint8_t *buffer = nullptr;
	int size = 0;
	Vector<uint8_t> data;
	if (p_peer->get_packet(&buffer, size) == OK) {
		data.resize(size);
		memcpy(data.ptrw(), buffer, size);
	}
	return data;
}

// One emission with the given arguments, in the format SIGNAL_CHECK expects.
static Array emission(const Array &p_args) {
	Array emissions;
	emissions.push_back(p_args);
	return emissions;
}

static Vector<uint8_t> bytes(std::initializer_list<uint8_t> p_values) {
	Vector<uint8_t> data;
	for (uint8_t value : p_values) {
		data.push_back(value);
	}
	return data;
}

TEST_CASE("[Steam][SteamMultiplayerPeer] Defaults") {
	Ref<SteamMultiplayerPeer> peer;
	peer.instantiate();
	CHECK(peer->get_connection_status() == MultiplayerPeer::CONNECTION_DISCONNECTED);
	CHECK(peer->get_unique_id() == 0);
	CHECK_FALSE(peer->is_server());
	CHECK(peer->is_server_relay_supported());
	CHECK(peer->get_available_packet_count() == 0);
	CHECK(peer->get_max_packet_size() == STEAM_NETWORKING_MAX_MESSAGE_SIZE - SteamMultiplayerPeer::DATA_HEADER_SIZE - SteamMultiplayerPeer::SEQUENCE_SIZE);
	CHECK(peer->get_peer_steam_id(1) == 0);
	CHECK(peer->get_peer_ping(1) == -1);
}

TEST_CASE("[Steam][SteamMultiplayerPeer] Fails cleanly without a Steam session") {
	Steam *steam = Steam::get_singleton();
	REQUIRE(steam != nullptr);
	if (steam->is_initialized()) {
		return; // Only meaningful without a running Steam client.
	}
	Ref<SteamMultiplayerPeer> peer;
	peer.instantiate();
	ERR_PRINT_OFF;
	CHECK(peer->create_host(0) != OK);
	CHECK(peer->create_client(HOST_ID) != OK);
	ERR_PRINT_ON;
	CHECK(peer->get_connection_status() == MultiplayerPeer::CONNECTION_DISCONNECTED);
	CHECK_FALSE(steam->create_lobby());
	CHECK_FALSE(steam->join_lobby(109775241000000000ULL));
	CHECK_FALSE(steam->request_lobby_list());
	CHECK(steam->get_lobby_members(109775241000000000ULL).is_empty());
	CHECK(steam->get_lobby_data(109775241000000000ULL, "name").is_empty());
	CHECK(steam->get_all_lobby_data(109775241000000000ULL).is_empty());
	CHECK(steam->get_lobby_owner(109775241000000000ULL) == 0);
}

TEST_CASE("[Steam][SteamMultiplayerPeer] Handshake assigns peer IDs") {
	FakeSteamNetwork network;
	Ref<SteamMultiplayerPeer> host = make_peer(network, HOST_ID);
	Ref<SteamMultiplayerPeer> client = make_peer(network, CLIENT_A_ID);

	REQUIRE(host->create_host(7, 4) == OK);
	CHECK(host->is_server());
	CHECK(host->get_unique_id() == 1);
	CHECK(host->get_connection_status() == MultiplayerPeer::CONNECTION_CONNECTED);

	SIGNAL_WATCH(host.ptr(), "peer_connected");
	REQUIRE(client->create_client(HOST_ID, 7) == OK);
	CHECK(client->get_connection_status() == MultiplayerPeer::CONNECTION_CONNECTING);
	CHECK(client->get_unique_id() == 0);

	poll_all({ host, client });

	CHECK(client->get_connection_status() == MultiplayerPeer::CONNECTION_CONNECTED);
	const int client_id = client->get_unique_id();
	CHECK(client_id > 1);
	CHECK_FALSE(client->is_server());

	Array host_args = emission(Array{ client_id });
	SIGNAL_CHECK("peer_connected", host_args);
	SIGNAL_UNWATCH(host.ptr(), "peer_connected");

	CHECK(host->get_peer_steam_id(client_id) == CLIENT_A_ID);
	CHECK(host->get_peer_id_for_steam_id(CLIENT_A_ID) == client_id);
	CHECK(client->get_peer_steam_id(1) == HOST_ID);
	CHECK(client->get_peer_id_for_steam_id(HOST_ID) == 1);
	CHECK(host->get_peer_ping(client_id) == 42);
	CHECK(host->get_peer_id_for_steam_id(CLIENT_B_ID) == 0);
}

TEST_CASE("[Steam][SteamMultiplayerPeer] Client reports failure when nobody listens") {
	FakeSteamNetwork network;
	Ref<SteamMultiplayerPeer> client = make_peer(network, CLIENT_A_ID);
	REQUIRE(client->create_client(HOST_ID) == OK);
	poll_all({ client });
	CHECK(client->get_connection_status() == MultiplayerPeer::CONNECTION_DISCONNECTED);
	CHECK(network.connections.is_empty());
}

TEST_CASE("[Steam][SteamMultiplayerPeer] Packets keep sender, channel and transfer mode") {
	FakeSteamNetwork network;
	Ref<SteamMultiplayerPeer> host = make_peer(network, HOST_ID);
	Ref<SteamMultiplayerPeer> client_a = make_peer(network, CLIENT_A_ID);
	Ref<SteamMultiplayerPeer> client_b = make_peer(network, CLIENT_B_ID);
	REQUIRE(host->create_host() == OK);
	REQUIRE(client_a->create_client(HOST_ID) == OK);
	REQUIRE(client_b->create_client(HOST_ID) == OK);
	poll_all({ host, client_a, client_b });
	REQUIRE(client_a->get_connection_status() == MultiplayerPeer::CONNECTION_CONNECTED);
	REQUIRE(client_b->get_connection_status() == MultiplayerPeer::CONNECTION_CONNECTED);
	const int a_id = client_a->get_unique_id();
	const int b_id = client_b->get_unique_id();
	CHECK(a_id != b_id);

	SUBCASE("Client to server") {
		client_a->set_transfer_channel(3);
		client_a->set_transfer_mode(MultiplayerPeer::TRANSFER_MODE_UNRELIABLE);
		CHECK(client_a->put_packet(bytes({ 1, 2, 3 }).ptr(), 3) == OK);
		poll_all({ host });
		REQUIRE(host->get_available_packet_count() == 1);
		CHECK(host->get_packet_peer() == a_id);
		CHECK(host->get_packet_channel() == 3);
		CHECK(host->get_packet_mode() == MultiplayerPeer::TRANSFER_MODE_UNRELIABLE);
		CHECK(read_packet(host) == bytes({ 1, 2, 3 }));
		CHECK(host->get_available_packet_count() == 0);
	}

	SUBCASE("Server broadcast, targeted and excluding sends") {
		host->set_transfer_mode(MultiplayerPeer::TRANSFER_MODE_RELIABLE);
		host->set_target_peer(MultiplayerPeer::TARGET_PEER_BROADCAST);
		CHECK(host->put_packet(bytes({ 9 }).ptr(), 1) == OK);
		host->set_target_peer(b_id);
		CHECK(host->put_packet(bytes({ 8 }).ptr(), 1) == OK);
		host->set_target_peer(-b_id);
		CHECK(host->put_packet(bytes({ 7 }).ptr(), 1) == OK);
		poll_all({ client_a, client_b });

		REQUIRE(client_a->get_available_packet_count() == 2);
		CHECK(client_a->get_packet_peer() == 1);
		CHECK(client_a->get_packet_mode() == MultiplayerPeer::TRANSFER_MODE_RELIABLE);
		CHECK(read_packet(client_a) == bytes({ 9 }));
		CHECK(read_packet(client_a) == bytes({ 7 }));

		REQUIRE(client_b->get_available_packet_count() == 2);
		CHECK(read_packet(client_b) == bytes({ 9 }));
		CHECK(read_packet(client_b) == bytes({ 8 }));

		ERR_PRINT_OFF;
		host->set_target_peer(12345);
		CHECK(host->put_packet(bytes({ 1 }).ptr(), 1) == ERR_INVALID_PARAMETER);
		ERR_PRINT_ON;
	}

	SUBCASE("Unreliable ordered drops late packets") {
		client_a->set_transfer_mode(MultiplayerPeer::TRANSFER_MODE_UNRELIABLE_ORDERED);
		client_a->set_transfer_channel(1);
		for (uint8_t i = 1; i <= 3; i++) {
			CHECK(client_a->put_packet(&i, 1) == OK);
		}
		// Deliver 3, 2, 1: only 3 is newer than everything before it.
		network.reverse_inbox(network.find_connection(HOST_ID));
		poll_all({ host });
		REQUIRE(host->get_available_packet_count() == 1);
		CHECK(host->get_packet_mode() == MultiplayerPeer::TRANSFER_MODE_UNRELIABLE_ORDERED);
		CHECK(read_packet(host) == bytes({ 3 }));

		const uint8_t newer = 4;
		CHECK(client_a->put_packet(&newer, 1) == OK);
		poll_all({ host });
		REQUIRE(host->get_available_packet_count() == 1);
		CHECK(read_packet(host) == bytes({ 4 }));
	}

	SUBCASE("Oversized packets are rejected") {
		Vector<uint8_t> big;
		big.resize(client_a->get_max_packet_size() + 1);
		ERR_PRINT_OFF;
		CHECK(client_a->put_packet(big.ptr(), big.size()) != OK);
		ERR_PRINT_ON;
		big.resize(client_a->get_max_packet_size());
		CHECK(client_a->put_packet(big.ptr(), big.size()) == OK);
	}
}

TEST_CASE("[Steam][SteamMultiplayerPeer] Host refuses connections when full") {
	FakeSteamNetwork network;
	Ref<SteamMultiplayerPeer> host = make_peer(network, HOST_ID);
	Ref<SteamMultiplayerPeer> client_a = make_peer(network, CLIENT_A_ID);
	Ref<SteamMultiplayerPeer> client_b = make_peer(network, CLIENT_B_ID);
	REQUIRE(host->create_host(0, 1) == OK);
	REQUIRE(client_a->create_client(HOST_ID) == OK);
	poll_all({ host, client_a });
	REQUIRE(client_a->get_connection_status() == MultiplayerPeer::CONNECTION_CONNECTED);

	REQUIRE(client_b->create_client(HOST_ID) == OK);
	poll_all({ host, client_a, client_b });
	CHECK(client_b->get_connection_status() == MultiplayerPeer::CONNECTION_DISCONNECTED);
	CHECK(client_a->get_connection_status() == MultiplayerPeer::CONNECTION_CONNECTED);

	host->set_refuse_new_connections(true);
	client_a->close();
	poll_all({ host });
	REQUIRE(client_b->create_client(HOST_ID) == OK);
	poll_all({ host, client_b });
	CHECK(client_b->get_connection_status() == MultiplayerPeer::CONNECTION_DISCONNECTED);
}

TEST_CASE("[Steam][SteamMultiplayerPeer] Disconnections are reported") {
	FakeSteamNetwork network;
	Ref<SteamMultiplayerPeer> host = make_peer(network, HOST_ID);
	Ref<SteamMultiplayerPeer> client = make_peer(network, CLIENT_A_ID);
	REQUIRE(host->create_host() == OK);
	REQUIRE(client->create_client(HOST_ID) == OK);
	poll_all({ host, client });
	const int client_id = client->get_unique_id();
	REQUIRE(client_id > 1);

	SUBCASE("Client leaves") {
		SIGNAL_WATCH(host.ptr(), "peer_disconnected");
		client->close();
		CHECK(client->get_connection_status() == MultiplayerPeer::CONNECTION_DISCONNECTED);
		poll_all({ host });
		Array args = emission(Array{ client_id });
		SIGNAL_CHECK("peer_disconnected", args);
		SIGNAL_UNWATCH(host.ptr(), "peer_disconnected");
		CHECK(host->get_peer_steam_id(client_id) == 0);
	}

	SUBCASE("Host kicks the client") {
		SIGNAL_WATCH(host.ptr(), "peer_disconnected");
		SIGNAL_WATCH(client.ptr(), "peer_disconnected");
		host->disconnect_peer(client_id);
		Array host_args = emission(Array{ client_id });
		SIGNAL_CHECK("peer_disconnected", host_args);
		poll_all({ client });
		Array client_args = emission(Array{ 1 });
		SIGNAL_CHECK("peer_disconnected", client_args);
		SIGNAL_UNWATCH(host.ptr(), "peer_disconnected");
		SIGNAL_UNWATCH(client.ptr(), "peer_disconnected");
		CHECK(client->get_connection_status() == MultiplayerPeer::CONNECTION_DISCONNECTED);
	}

	SUBCASE("Host shuts down") {
		host->close();
		CHECK(host->get_connection_status() == MultiplayerPeer::CONNECTION_DISCONNECTED);
		poll_all({ client });
		CHECK(client->get_connection_status() == MultiplayerPeer::CONNECTION_DISCONNECTED);
		CHECK(network.connections.is_empty());
	}
}

#ifdef MODULE_MULTIPLAYER_ENABLED
TEST_CASE("[Steam][SteamMultiplayerPeer][SceneTree] SceneMultiplayer relays between clients") {
	FakeSteamNetwork network;
	Ref<SteamMultiplayerPeer> host = make_peer(network, HOST_ID);
	Ref<SteamMultiplayerPeer> client_a = make_peer(network, CLIENT_A_ID);
	Ref<SteamMultiplayerPeer> client_b = make_peer(network, CLIENT_B_ID);
	REQUIRE(host->create_host() == OK);
	REQUIRE(client_a->create_client(HOST_ID) == OK);
	REQUIRE(client_b->create_client(HOST_ID) == OK);

	Ref<SceneMultiplayer> host_mp;
	host_mp.instantiate();
	Ref<SceneMultiplayer> a_mp;
	a_mp.instantiate();
	Ref<SceneMultiplayer> b_mp;
	b_mp.instantiate();
	host_mp->set_root_path(NodePath("/root"));
	a_mp->set_root_path(NodePath("/root"));
	b_mp->set_root_path(NodePath("/root"));
	host_mp->set_multiplayer_peer(host);
	a_mp->set_multiplayer_peer(client_a);
	b_mp->set_multiplayer_peer(client_b);

	SIGNAL_WATCH(a_mp.ptr(), "connected_to_server");
	for (int i = 0; i < 8; i++) {
		host_mp->poll();
		a_mp->poll();
		b_mp->poll();
	}
	SIGNAL_CHECK("connected_to_server", emission(Array()));
	SIGNAL_UNWATCH(a_mp.ptr(), "connected_to_server");

	const int a_id = a_mp->get_unique_id();
	const int b_id = b_mp->get_unique_id();
	REQUIRE(a_id > 1);
	REQUIRE(b_id > 1);
	// Each client learns about the other through the host.
	CHECK(a_mp->get_peer_ids().has(b_id));
	CHECK(b_mp->get_peer_ids().has(a_id));
	CHECK(host_mp->get_peer_ids().size() == 2);

	SIGNAL_WATCH(b_mp.ptr(), "peer_packet");
	PackedByteArray payload;
	payload.push_back(42);
	payload.push_back(7);
	CHECK(a_mp->send_bytes(payload, b_id) == OK);
	for (int i = 0; i < 4; i++) {
		host_mp->poll();
		b_mp->poll();
	}
	Array expected = emission(Array{ a_id, payload });
	SIGNAL_CHECK("peer_packet", expected);
	SIGNAL_UNWATCH(b_mp.ptr(), "peer_packet");

	SIGNAL_WATCH(b_mp.ptr(), "peer_disconnected");
	a_mp->disconnect_peer(1);
	for (int i = 0; i < 4; i++) {
		host_mp->poll();
		b_mp->poll();
	}
	Array disconnected = emission(Array{ a_id });
	SIGNAL_CHECK("peer_disconnected", disconnected);
	SIGNAL_UNWATCH(b_mp.ptr(), "peer_disconnected");

	host_mp->set_multiplayer_peer(Ref<MultiplayerPeer>());
	a_mp->set_multiplayer_peer(Ref<MultiplayerPeer>());
	b_mp->set_multiplayer_peer(Ref<MultiplayerPeer>());
}
#endif

} // namespace TestSteamMultiplayerPeer
