#if defined(__USE_SPIDERMONKEY__)
#pragma comment(lib, "spidermonkey.lib")
#elif defined(__USE_V8__)
#pragma comment(lib, "v8.lib")
#endif

#ifndef __SCRIPTENGINE_H__
#define __SCRIPTENGINE_H__

#include <windows.h>
#include <map>
#include <list>
#include <string>

#include "Script.h"
#include "yasper.h"

namespace botsys
{

typedef std::map<std::string, ScriptPtr> ScriptMap;
typedef std::list<ScriptPtr> ScriptList;

enum EngineState
{
	NotRunning,
	Starting,
	Started,
	Stopping,
	Stopped
};

class ScriptEngine
{
private:
	static CRITICAL_SECTION lock;
	static EngineState state;
	static ScriptMap scripts;
	static std::string scriptPath;
	static void* pData;

	// disable the default ctor, the copy ctor, and the assignment op
	ScriptEngine();
	~ScriptEngine();
	ScriptEngine(const ScriptEngine&);
	ScriptEngine& operator=(const ScriptEngine&);

public:
	// Compile needs to ensure that it adds the script to the list
	static ScriptPtr Compile(std::string filename, bool recompile = false);

	static ScriptPtr Find(std::string filename)
	{
		return scripts.count(filename) ? scripts[filename] : NULL;
	}
	static ScriptPtr FindOrCompile(std::string filename, bool recompile = false)
	{
		Script* result = Find(filename);
		if(!result)
			result = Compile(filename, recompile);
		return result;
	}
	static void Release(ScriptPtr script)
	{
		script->End();

		// make sure we have exclusive access to the list while we fiddle with it
		EnterCriticalSection(&lock);
		scripts.erase(script->GetFilename());
		LeaveCriticalSection(&lock);
	}

	static void GetScripts(ScriptList& list)
	{
		// clear the (possibly tainted) list
		list.clear();

		// make sure we have exclusive access to the list while we copy it
		EnterCriticalSection(&lock);

		for(ScriptMap::iterator it = scripts.begin(); it != scripts.end(); it++)
			list.push_back(it->second);

		LeaveCriticalSection(&lock);
	}

	// intentionally leaving this as a ptr
	static void* GetPrivateData(void) { return pData; }
	static void SetPrivateData(void* data) { EnterCriticalSection(&lock); pData = data; LeaveCriticalSection(&lock); }

	static void Startup(std::string path, unsigned long maxBytes);
	static void Shutdown(void);
};

}

#endif