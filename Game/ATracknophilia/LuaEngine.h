#pragma once
#include <iostream>
#include <vector>
#include "Singleton.h"

extern "C"
{
	#include <../dependancies/lua51/lua.h>
	#include <../dependancies/lua51/lauxlib.h>
	#include <../dependancies/lua51/lualib.h>
}

#include <../dependancies/LuaBridge/LuaBridge.h>

class LuaEngine : public Singleton<LuaEngine>
{
public:
	//Constructor
	LuaEngine(void) : m_L(luaL_newstate()) { luaL_openlibs(m_L); }
	LuaEngine(const LuaEngine& other);  //non-construction copy
	LuaEngine& operator=(const LuaEngine&); //non-copy
	~LuaEngine(void) { lua_close(m_L); } //Destructor clean-up
										 
	lua_State* L(); //returns lua state object

	//Execute Lua File
	void ExecuteFile(const char* file);

	luabridge::LuaRef ExecuteFunction(const char* fnName);

	void LoadScripts(std::vector<const char*> files);

	//Execute Lua Expression (contained in string)
	void ExecuteString(const char* expression);

private:
	//member lua state
	lua_State* m_L;

private:

	//handle errors
	void report_errors(int state);
};