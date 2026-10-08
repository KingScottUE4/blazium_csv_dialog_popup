/**************************************************************************/
/*  steam_multiplayer_peer.h                                              */
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

#include "steam_types.h"

#include "core/templates/hash_map.h"
#include "core/templates/list.h"
#include "core/templates/local_vector.h"
#include "scene/main/multiplayer_peer.h"

class SteamMultiplayerPeer;

// A connection state change reported by Steam Networking Sockets
// (SteamNetConnectionStatusChangedCallback_t), reduced to what the peer needs.
struct SteamNetConnectionEvent {
	SteamHSteamNetConnection connection = STEAM_NET_CONNECTION_INVALID;
	SteamHSteamListenSocket listen_socket = STEAM_LISTEN_SOCKET_INVALID;
	uint64_t remote_steam_id = 0;
	int old_state = STEAM_NET_CONNECTION_STATE_NONE;
	int state = STEAM_NET_CONNECTION_STATE_NONE;
	int end_reason = 0;
	String end_debug;
};

// The subset of ISteamNetworkingSockets used by SteamMultiplayerPeer. The
// default implementation talks to Steam; tests substitute an in-memory one.
class SteamNetworkingTransport {
public:
	struct Message {
		SteamHSteamNetConnection connection = STEAM_NET_CONNECTION_INVALID;
		Vector<uint8_t> data;
	};

protected:
	SteamMultiplayerPeer *peer = nullptr;

public:
	void set_peer(SteamMultiplayerPeer *p_peer) { peer = p_peer; }

	virtual Error open() = 0;
	virtual void close() = 0;
	virtual SteamHSteamListenSocket create_listen_socket(int p_virtual_port) = 0;
	virtual void close_listen_socket(SteamHSteamListenSocket p_socket) = 0;
	virtual SteamHSteamNetConnection connect(uint64_t p_steam_id, int p_virtual_port) = 0;
	virtual bool accept_connection(SteamHSteamNetConnection p_connection) = 0;
	virtual void close_connection(SteamHSteamNetConnection p_connection, int p_reason, const String &p_debug, bool p_linger) = 0;
	virtual Error send_message(SteamHSteamNetConnection p_connection, const uint8_t *p_data, int p_size, bool p_reliable) = 0;
	virtual void receive_messages(List<Message> &r_messages) = 0;
	virtual int get_ping(SteamHSteamNetConnection p_connection) const = 0;
	// Pumps Steam callbacks so connection events reach the peer.
	virtual void poll() = 0;

	virtual ~SteamNetworkingTransport() {}
};

// SteamNetworkingTransport backed by the Steam client (ISteamNetworkingSockets).
class SteamSocketsTransport : public SteamNetworkingTransport {
	SteamHSteamNetPollGroup poll_group = STEAM_NET_POLL_GROUP_INVALID;

public:
	void on_connection_status_changed(const SteamNetConnectionEvent &p_event);
	void on_steam_shutdown();

	virtual Error open() override;
	virtual void close() override;
	virtual SteamHSteamListenSocket create_listen_socket(int p_virtual_port) override;
	virtual void close_listen_socket(SteamHSteamListenSocket p_socket) override;
	virtual SteamHSteamNetConnection connect(uint64_t p_steam_id, int p_virtual_port) override;
	virtual bool accept_connection(SteamHSteamNetConnection p_connection) override;
	virtual void close_connection(SteamHSteamNetConnection p_connection, int p_reason, const String &p_debug, bool p_linger) override;
	virtual Error send_message(SteamHSteamNetConnection p_connection, const uint8_t *p_data, int p_size, bool p_reliable) override;
	virtual void receive_messages(List<Message> &r_messages) override;
	virtual int get_ping(SteamHSteamNetConnection p_connection) const override;
	virtual void poll() override;

	SteamSocketsTransport();
	~SteamSocketsTransport();
};

class SteamMultiplayerPeer : public MultiplayerPeer {
	GDCLASS(SteamMultiplayerPeer, MultiplayerPeer);

public:
	// Wire format version, bumped on incompatible protocol changes.
	static constexpr uint8_t PROTOCOL_VERSION = 1;

	enum MessageType {
		MESSAGE_DATA = 0,
		MESSAGE_ASSIGN_ID = 1,
	};

	static constexpr int DATA_HEADER_SIZE = 3; // type, transfer mode, channel.
	static constexpr int SEQUENCE_SIZE = 4; // Extra header for TRANSFER_MODE_UNRELIABLE_ORDERED.
	static constexpr int ASSIGN_ID_SIZE = 6; // type, protocol version, int32 id.

private:
	enum Mode {
		MODE_NONE,
		MODE_SERVER,
		MODE_CLIENT,
	};

	struct Packet {
		int from = 0;
		int channel = 0;
		TransferMode transfer_mode = TRANSFER_MODE_RELIABLE;
		Vector<uint8_t> data;
	};

	struct RemotePeer {
		SteamHSteamNetConnection connection = STEAM_NET_CONNECTION_INVALID;
		uint64_t steam_id = 0;
	};

	SteamNetworkingTransport *transport = nullptr;
	Mode mode = MODE_NONE;
	ConnectionStatus connection_status = CONNECTION_DISCONNECTED;
	int unique_id = 0;
	int target_peer = 0;
	int max_clients = 0;
	SteamHSteamListenSocket listen_socket = STEAM_LISTEN_SOCKET_INVALID;
	SteamHSteamNetConnection server_connection = STEAM_NET_CONNECTION_INVALID;
	uint64_t server_steam_id = 0;

	HashMap<int, RemotePeer> peers; // Connected peers by peer ID.
	HashMap<SteamHSteamNetConnection, int> connection_to_peer;
	HashMap<SteamHSteamNetConnection, uint64_t> pending_connections; // Accepted, waiting for Connected.

	List<SteamNetConnectionEvent> pending_events;
	List<Packet> incoming_packets;
	Packet current_packet;

	HashMap<int, uint32_t> ordered_send_sequence; // By channel.
	HashMap<uint64_t, uint32_t> ordered_receive_sequence; // By (connection << 32 | channel).

	SteamNetworkingTransport *_get_transport();
	void _process_event(const SteamNetConnectionEvent &p_event);
	void _process_server_event(const SteamNetConnectionEvent &p_event);
	void _process_client_event(const SteamNetConnectionEvent &p_event);
	void _process_message(const SteamNetworkingTransport::Message &p_message);
	void _drop_connection(SteamHSteamNetConnection p_connection);
	Error _send_to(SteamHSteamNetConnection p_connection, const uint8_t *p_data, int p_size, bool p_reliable);
	void _reset();

protected:
	static void _bind_methods();

public:
	Error create_host(int p_virtual_port = 0, int p_max_clients = 32);
	Error create_client(uint64_t p_host_steam_id, int p_virtual_port = 0);

	uint64_t get_peer_steam_id(int p_peer_id) const;
	int get_peer_id_for_steam_id(uint64_t p_steam_id) const;
	int get_peer_ping(int p_peer_id) const;
	int get_max_clients() const { return max_clients; }

	// MultiplayerPeer.
	virtual void set_target_peer(int p_peer_id) override;
	virtual int get_packet_peer() const override;
	virtual TransferMode get_packet_mode() const override;
	virtual int get_packet_channel() const override;
	virtual void disconnect_peer(int p_peer, bool p_force = false) override;
	virtual bool is_server() const override;
	virtual void poll() override;
	virtual void close() override;
	virtual int get_unique_id() const override;
	virtual ConnectionStatus get_connection_status() const override;
	virtual bool is_server_relay_supported() const override { return true; }

	// PacketPeer.
	virtual int get_available_packet_count() const override;
	virtual Error get_packet(const uint8_t **r_buffer, int &r_buffer_size) override;
	virtual Error put_packet(const uint8_t *p_buffer, int p_buffer_size) override;
	virtual int get_max_packet_size() const override;

	// Internal: takes ownership of a custom transport (used by tests).
	void set_transport(SteamNetworkingTransport *p_transport);
	// Internal: called by the transport when Steam reports a state change.
	void queue_connection_event(const SteamNetConnectionEvent &p_event);

	SteamMultiplayerPeer();
	~SteamMultiplayerPeer();
};
