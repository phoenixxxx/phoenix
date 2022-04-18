#pragma once
#include "Immobile.h"

#define SINGLETON_METHODS(Name)\
	IMMOBILE_METHODS(Name)\
public:\
	static Name* Instance() {\
		static Name myInstance;\
		return &myInstance;\
	}\
protected:\
	Name() {\
	}\
	~Name() {\
	}\
private:\
