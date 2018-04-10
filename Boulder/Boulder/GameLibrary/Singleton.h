#ifndef SINGLETON_H
#define SINGLETON_H

#include <SFML/System.hpp>

// forward declaration of singleton class
template <class T> class Singleton;

// the singleton access class
// to control access to the singleton object and
// protect against multithreading problems with a 
// mutual exclusion lock
template <class T> class SingletonAccess {
	friend class Singleton<T>;
	static sf::Mutex mutex;

	T* t;
	// A RAII helper for the mutex (locks on construction and unlocks on destruction)
	sf::Lock lock;

	SingletonAccess(T* t) : t(t), lock(mutex) {} // give the access class a pointer to the singleton
	SingletonAccess operator=(const SingletonAccess&); // no assignment operator
public:
	SingletonAccess(const SingletonAccess& sa) : t(sa.t) {} // default copy constructor (@Disch do I need this? or is it implied)

	T* operator->()
	{
		return t;
	}
};

// the singleton itself
template <class T> class Singleton {
public:
	// get a singleton access object to allow thread safe access
	// to the singleton object
	static SingletonAccess<T> Get()
	{
		static T t;
		return SingletonAccess<T>(&t);
	}
};

template <class T> sf::Mutex SingletonAccess<T>::mutex;

#endif