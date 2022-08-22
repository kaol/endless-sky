/* Dictionary.cpp
Copyright (c) 2017 by Michael Zahniser

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#include "Dictionary.h"

#include <cstring>
#include <map>
#include <mutex>
#include <string>

using namespace std;

namespace {
	static map<string, int> nameToKey;
	static vector<const char *> names;

	// Perform a binary search on a sorted vector. Return the key's location (or
	// proper insertion spot) in the first element of the pair, and "true" in
	// the second element if the key is already in the vector.
	/*
	pair<size_t, bool> Search(const char *key, const vector<pair<const char *, double>> &v)
	{
		// At each step of the search, we know the key is in [low, high).
		size_t low = 0;
		size_t high = v.size();

		while(low != high)
		{
			size_t mid = (low + high) / 2;
			int cmp = strcmp(key, v[mid].first);
			if(!cmp)
				return make_pair(mid, true);

			if(cmp < 0)
				high = mid;
			else
				low = mid + 1;
		}
		return make_pair(low, false);
	}
	*/

	static int newName(const string &name)
	{
		static map<string, int> dummy;
		static mutex m;

		lock_guard<mutex> lock(m);
		int key = names.size();
		char *copy = strndup(name.c_str(), 255);
		names.push_back(copy);
		nameToKey.emplace(name, key);
		return key;
	}
}



/*
double &Dictionary::operator[](const char *name)
{
	// fix
	static mutex m;

	lock_guard<mutex> lock(m);
	int index = GetKey(name);
	if(index >= int((*this).size()))
		(*this).resize(index+1);
	return data()[index];
}
*/



double &Dictionary::operator[](const string &name)
{
	int index = GetKey(name);
	if(index >= int((*this).size()))
		(*this).resize(index+1);
	return data()[index];
	//return (*this)[key.c_str()];
}



/*
double &Dictionary::operator[](const int &key)
{
	//return data()[key];
	double &value = (*this)[vector<double>::size_type(key)];
	return value;
}
*/



/*
double Dictionary::Get(const char *key) const
{
	return Get(
	auto it = nameToKey.find(key);
	if(it == nameToKey.cend() || int((*this).size()-1) < it->second)
		return 0.;
	else
		return at(it->second);
}



double Dictionary::Get(const string &key) const
{
	return Get(key.c_str());
}
*/
double Dictionary::Get(const char *key) const
{
	return Get(string(key));
}



double Dictionary::Get(const string &key) const
{
	auto it = nameToKey.find(key);
	if(it == nameToKey.cend() || int((*this).size()-1) < it->second)
		return 0.;
	else
		return at(it->second);
}



double Dictionary::Get(const int &key) const
{
	if(key < 0 || key >= int(this->size()))
		return 0.;
	return this->at(key);
}



void Dictionary::Grow()
{
	resize(names.size()+1);
}



const char *Dictionary::GetName(const int key)
{
	if(key < 0 || key >= int(names.size()))
		return nullptr;
	return names.at(key);
}



// Inserts the name if not found.
const int Dictionary::GetKey(const string &name)
{
	map<string, int>::iterator it = nameToKey.find(name);
	if(it == nameToKey.end())
		return newName(name.c_str());
	else
		return it->second;
}



const int Dictionary::GetKey(const char *name)
{
	return GetKey(string(name));
}



const int Dictionary::LookupKey(const string &name)
{
	map<string, int>::const_iterator it = nameToKey.find(name);
	if(it == nameToKey.end())
		return -1;
	else
		return it->second;
}
