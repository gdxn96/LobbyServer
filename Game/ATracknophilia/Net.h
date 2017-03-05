#pragma once
#include "NetworkAdapter.h"

struct Lobby
{
	META_DATA(Lobby);
	int id;
	int num_players;
	int max_players;
};

DEFINE_META(Lobby)
{
	ADD_MEMBER(id);
	ADD_MEMBER(num_players);
	ADD_MEMBER(max_players);
};

class Net
{
public:
	Net() : m_net("http://127.0.0.1:5001/")
	{
		m_net.initUDPConnection("127.0.0.1", 8888, 5226);
	}

	void update()
	{
		m_net.getMessages();
	}

	std::vector<Lobby> getLobbies()
	{
		std::vector<Lobby> lobbies;
		const char * data = m_net.serverGet("lobbies").body;

		rapidjson::Document d;
		d.Parse(data);
		assert(d.IsObject());
		for (auto& lobby : d["lobbies"].GetArray())
		{
			Lobby l;
			Variable(&l).FromJson<Lobby>(lobby.GetString());
			lobbies.push_back(l);
		}
		return lobbies;
	}

	bool joinLobbyById(int lobbyId)
	{
		auto response = m_net.serverGet((std::string("lobbies/join/") + std::to_string(lobbyId)).c_str());
		return response.valid();
	}

	bool leaveLobby()
	{
		auto response = m_net.serverGet("lobbies/leave/");
		return response.valid();
	}

	bool hostNewLobby()
	{
		auto response = m_net.serverGet((std::string("lobbies/new/") + std::to_string(4)).c_str());//maxplayers
		return response.valid();
	}

	void sendToHost(Message msg)
	{
		m_net.udpSend("send_to_host", msg);
	}

	void sendToAll(Message msg)
	{
		m_net.udpSend("send_to_all", msg);
	}

	void setMessageCallback(std::function<void(Message)> messageCallback)
	{
		m_net.setMessageCallback(messageCallback);
	}

private:
	NetworkAdapter m_net;
};
