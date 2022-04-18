#pragma once
#define IMMOBILE_METHODS(Name)\
public:\
	Name(Name const&) = delete;\
	Name(Name&&) = delete;\
	Name& operator=(Name const&) = delete;\
	Name& operator=(Name&&) = delete;\
private:\
