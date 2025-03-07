/**
 *  This file is part of the EGIL SCIM client.
 *
 *  Copyright (C) 2017-2019 Föreningen Sambruk
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.

 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.

 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef SIMPLESCIM_USER_H
#define SIMPLESCIM_USER_H

#include <stddef.h>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <ostream>
#include <iostream>
#include <algorithm>
#include <optional>

using string_pair = std::pair<std::string, std::string>;
using attrib_name = std::string;
using string_vector = std::vector<std::string>;
using attrib_map = std::map<attrib_name, string_vector>;

class base_object {
	attrib_map attributes{};

	mutable std::string identity;
	mutable std::string ss12000type;

	string_vector empty{};

public:
	friend class cache_file;

	base_object() = delete;

	explicit base_object(const std::string &type) : ss12000type(type) {
		attributes.emplace(std::make_pair("ss12000type", string_vector({type})));
	}
	base_object(const attrib_name &attr, const string_vector &values) {
		add_attribute(attr, values);
	}

	base_object(const attrib_name &attr, string_vector &&values) {
		add_attribute(attr, std::move(values));
	}

	explicit base_object(attrib_map &&data) : attributes(std::move(data)) {}

	base_object(const base_object &other) {
		*this = other;
	}

	base_object(base_object &&other) noexcept {
		*this = std::move(other);
	}

	size_t size() {
		return attributes.size();
	}
	bool has_attribute_with_value(const std::string& a, const std::string& v) {
		auto values = attributes.find(a);
		if (values != attributes.end()) {
			for (auto value : values->second) {
				if (value == v)
					return true;
			}
		}
		return false;
	}

	bool has_attribute_or_relation(const std::string& attr);

	attrib_map::const_iterator begin() const {
		return attributes.begin();
	}

	attrib_map::const_iterator end() const {
		return attributes.end();
	}

	friend std::ostream &operator<<(std::ostream &os, const base_object &object);

	base_object &operator=(base_object &&other) noexcept {
		identity = std::move(other.identity);
		attributes = std::move(other.attributes);
		ss12000type = std::move(other.ss12000type);
		return *this;
	}

	base_object &operator=(const base_object &other) {
		identity = other.identity;
		attributes = other.attributes;
		ss12000type = other.ss12000type;
		return *this;
	}

	std::string get_uid(bool search = true) const;

	std::string getSS12000type() const {
		const string_vector list = get_values("ss12000type");
		if (!list.empty()) {
			return list.at(0);
		}
		std::cerr << "base_object::getSS12000type missing type" << std::endl;
		return "";
	}

	const string_vector &get_values(const std::string &attr) const {
		auto record = attributes.find(attr);
		if (record != attributes.end()) {
			return record->second;
		}
		return empty;
	}

	void sortAttribute(std::string a) {
		auto list = attributes.find(a);
		if (list != attributes.end())
			std::sort(list->second.begin(), list->second.end());
	}

	void append_values(const std::string &attr, const string_vector &values, bool unique = false) {
		auto iter = attributes.find(attr);
		if (iter != attributes.end()) {
			if (unique) {
				for (auto &&val: values) {
					if (std::find(std::begin(iter->second), std::end(iter->second), val) == std::end(iter->second))
						iter->second.emplace_back(val);
				}
			} else
				iter->second.insert(iter->second.end(), values.begin(), values.end());
		} else
			attributes.emplace(std::make_pair(attr, values));
	}

	void append_values(const std::string &attr, string_vector &&values, bool unique = false) {
		auto iter = attributes.find(attr);
		if (iter != attributes.end()) {
			if (unique) {
				for (auto &&val: values) {
					if (std::find(std::begin(iter->second), std::end(iter->second), val) == std::end(iter->second))
						iter->second.emplace_back(std::move(val));
				}
			} else
				iter->second.insert(iter->second.end(), values.begin(), values.end());
		} else
			attributes.emplace(std::make_pair(attr, std::move(values)));
	}

	void add_attribute(const std::string &attr, const string_vector &values) {
		auto iter = attributes.find(attr);

		if (iter != attributes.end()) {
			attributes.erase(attr);
		}
		attributes.emplace(std::make_pair(attr, values));
	}

	void add_attribute(const std::string &attr, string_vector &&values) {
		auto iter = attributes.find(attr);

		if (iter != attributes.end()) {
			attributes.erase(attr);
		}
		attributes.emplace(std::make_pair(attr, std::move(values)));
	}

	size_t number_of_attributes() const {
		return attributes.size();
	}

	bool operator==(const base_object &rhs) const {
       return attributes == rhs.attributes;
	}

};

/**
 * Constructs a base_object from a vector of values.
 * 
 * Used by the CSV and SQL load processes.
 */
std::shared_ptr<base_object> vector_to_base_object(const std::vector<std::optional<std::string>> &values,
												   const string_vector& attribute_names,
												   const std::string& type);

/**
 * Generates a UUID attribute for an object, based on some other attribute
 */
void generate_uuid(std::shared_ptr<base_object> object,
				   const std::string &generator,
				   const std::string &uuid_attribute);

#endif
