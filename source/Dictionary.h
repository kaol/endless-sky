/* Dictionary.h
Copyright (c) 2017 by Michael Zahniser

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#ifndef DICTIONARY_H_
#define DICTIONARY_H_

#include <mutex>
#include <string>
#include <utility>
#include <vector>



class Dictionary : private std::vector<double> {
public:
	// Access a key for modifying it:
	//double &operator[](const char *name);
	double &operator[](const std::string &name);
	double &operator[](const int &key);
	// Get the value of a key, or 0 if it does not exist:
	double Get(const char *key) const;
	double Get(const std::string &key) const;
	double Get(const int &key) const;

	// Make the vector large enough to fit every attribute.
	void Grow();

	static const char *GetName(const int key);
	static const int GetKey(const char *name);
	static const int GetKey(const std::string &name);
	static const int LookupKey(const std::string &name);

	// Expose certain functions from the underlying vector:
	using std::vector<double>::operator[];
	using std::vector<double>::empty;
	using std::vector<double>::begin;
	using std::vector<double>::end;
};



#endif
