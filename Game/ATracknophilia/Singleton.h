#pragma once

template<typename T>
class Singleton
{
public:
	static T* getInstance() 
	{
		if (!m_instance)
		{
			m_instance = new T();
		}
		return m_instance;
	}

private:
	static T* m_instance;
};

template<typename T>
T* Singleton<T>::m_instance = nullptr;
