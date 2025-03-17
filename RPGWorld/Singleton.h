#pragma once

template <typename T>
class Singleton
{
public:
	static T* GetInstance()
	{
		if (_instance == nullptr)
		{
			_instance = new T;
		}

		return _instance;
	}

	static void DestroyInstance()
	{
		if (_instance != nullptr)
		{
			delete _instance;
			_instance = nullptr;
		}	
	}

protected:
	Singleton()								= default;

private:
	Singleton(const Singleton&)				= delete;
	Singleton(Singleton&)					= delete;
	Singleton(Singleton&&)					= delete;

	Singleton& operator=(const Singleton&)	= delete;
	Singleton& operator=(Singleton&)		= delete;
	Singleton& operator=(Singleton&&)		= delete;

	static T* _instance;
};

template <typename T> T* Singleton<T>::_instance = nullptr;