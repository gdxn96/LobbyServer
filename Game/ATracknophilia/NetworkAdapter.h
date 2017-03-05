#pragma once
#include "LuaEngine.h"
#include "../dependancies/MetaSystem/MetaSystem.h"
#include <functional>

#define INCOMING_MESSAGE(x) namespace MessageType {	namespace INCOMING { static const char* x = #x;	}	}
#define OUTGOING_MESSAGE(x) namespace MessageType {	namespace OUTGOING { static const char* x = #x;	}	}

INCOMING_MESSAGE(ping)
OUTGOING_MESSAGE(pong)

struct Message
{
	META_DATA(Message);
	std::string message_type = "";
	std::string data;
	Message(const char * _data, const char* _messageType) : data(_data), message_type(_messageType) {}
	
	Message(const char * json) {
		Variable(this).FromJson<Message>(json);
	}
	const char* toJson()
	{
		return Variable(this).ToJson().c_str();
	}
};

DEFINE_META(Message)
{
	ADD_MEMBER(message_type);
	ADD_MEMBER(data);
}

class REST_Response
{
public:
	int code;
	const char* body;

	REST_Response(int rc, const char * rb) :code(rc), body(rb)
	{

	}

	bool valid()
	{
		return code > 199 && code < 300;
	}
};

class NetworkAdapter
{
public:
	NetworkAdapter(const char * httpServerURI) : connected(false), m_serverUri(httpServerURI), m_messageCallback(nullptr)
	{
		m_lua = LuaEngine::getInstance();
		//allow lua to create c++ obejcts
		luabridge::getGlobalNamespace(m_lua->L())
		.beginNamespace("CPP")
			.beginClass<REST_Response>("REST_Response")
			.addConstructor<void(*)(int, const char*)>()
			.endClass()
		.endNamespace();

		luabridge::getGlobalNamespace(m_lua->L())
		.beginNamespace("CPP")
			.beginClass<Message>("Message")
			.addConstructor<void(*)(const char*, const char *)>()
			//.addConstructor<void(*)(const char*)>()
			.addFunction("toJson", &Message::toJson)
			.endClass()
		.endNamespace();

		luabridge::getGlobalNamespace(m_lua->L())
		.beginNamespace("CPP")
			.beginClass<NetworkAdapter>("NetworkAdapter")
			.addFunction("notifyMessage", &NetworkAdapter::notifyMessage)
			.endClass()
		.endNamespace();

		// load/execute script
		m_lua->LoadScripts({
			"networking.lua"
		});		
	}

	void initUDPConnection(const char * serverIp, int serverPort, int clientPort)
	{
		m_lua->ExecuteFunction("udp_setup")(serverIp, serverPort, clientPort);
	}

	void setMessageCallback(std::function<void(Message)> messageCallback)
	{
		m_messageCallback = messageCallback;
	}

	~NetworkAdapter() 
	{
		m_lua->ExecuteFunction("disconnect")();
	}

	void notifyMessage(const char* json)
	{
		if (json)
		{
			Message m(json);
			if (m_messageCallback)
			{
				m_messageCallback(m);
			}

			if (m.message_type == "pong")
			{
				connected = true;
			}
		}
	}

	void udpSend(const char* data, Message message)
	{
		assert(connected);
		m_lua->ExecuteFunction("udp_send")(data, message.toJson());
	}

	void getMessages()
	{
		m_lua->ExecuteFunction("receive_messages")(this);
	}

	REST_Response serverGet(const char* uriPath)
	{
		return m_lua->ExecuteFunction("get_request")(std::string(m_serverUri) + uriPath);
	}

	REST_Response serverPost(const char* uriPath, const char * postData)
	{
		return m_lua->ExecuteFunction("post_request")(std::string(m_serverUri) + uriPath, postData);
	}

	bool connected;
private:
	LuaEngine* m_lua;
	const char * m_serverUri;
	std::function<void(Message)> m_messageCallback;
};
