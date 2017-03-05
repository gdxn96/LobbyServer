#pragma once
#include "AutoLister.h"
#include "Meta.h"

//inherits from AutoLister, this is how we update all objects when object json is updated
template<typename T>
class RuntimeEditable : public AutoLister<T>
{
public: 
	virtual void jsonInit() { Variable(static_cast<T *>(this)).FromJson<T>(JsonHandler::get<T>(), true); };

	static void reloadJson()
	{
		auto& list = AutoList::get<T>();
		JsonHandler::get<T>("", true);
		for (T* object : list)
		{
			object->jsonInit();
		}
	}

	virtual ~RuntimeEditable() {};
};
