-- load the http module
local http = require'socket.http'
local ltn12 = require"ltn12"

local udp = assert(socket.udp())

function  udp_setup( SERVER_IP, SERVER_PORT, CLIENT_PORT )
	udp:setsockname('*', CLIENT_PORT)
	udp:setpeername( SERVER_IP, SERVER_PORT )

	local ip, sock = udp:getsockname()
	print( "myIP:", ip, sock )

	udp:settimeout(0)
	local ping = CPP.Message('', 'ping')
	udp:send(ping:toJson())
	print("ping sent")
	print(ping:toJson())
end

function receive_messages(network_adapter)
	repeat
		data, msg = udp:receive()
		
		--if msg then print(msg) end
		if data then 
			print(data) 
			network_adapter:notifyMessage(data)
		elseif msg ~= 'timeout' then 
			error("Network error: "..tostring(msg))
		end
	until not data 
end

function udp_send(data, message_type)
	udp:send(CPP.Message(data, message_type):toJson())
end

function post_request(url, POST_DATA)
	local respbody = {}

	-- Requests information about a document, without downloading it.
	-- Useful, for example, if you want to display a download gauge and need
	-- to know the size of the document in advance
	r, s, c = http.request {
	  method = "POST",
	  url = url,
	  sink = ltn12.sink.table(respbody),
	  headers = {
	            ["content-type"] = "application/json",
	            ["content-length"] = tostring(#POST_DATA)
	        }
	}

	-- get body as string by concatenating table filled by sink
	respbody = table.concat(respbody)
	local rest_response = CPP.REST_Response(s, respbody)


	return rest_response
end

function get_request(url)
	print"get_request"
	local respbody = {}

	r, s, c = http.request {
	  method = "GET",
	  url = url,
	  sink = ltn12.sink.table(respbody)
	}

	-- get body as string by concatenating table filled by sink
	respbody = table.concat(respbody)

	print(respbody)
	local rest_response = CPP.REST_Response(s, respbody)

	return rest_response
end
