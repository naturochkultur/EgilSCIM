//
// Created by Ola Mattsson on 2018-09-20.
//

#include "data_server.hpp"
#include "simplescim_ldap.hpp"
#include "json_data_file.hpp"

/**
 * load all data from
 * store each type in the data map with the type as key
 */
void data_server::load() {
	config_file &config = config_file::instance();
	static_types = string_to_vector(config.get("scim-static-types"));
	dynamic_types = string_to_vector(config.get("scim-dynamic-types"));

	std::shared_ptr<object_list> all = std::make_shared<object_list>();
	string_vector types = config.get_vector("scim-type-load-order");
	ldap_wrapper ldap;
	if (ldap.valid()) {
		for (const auto &type : types) {
			std::string sourceType = config.get(type + "-scim-data-source");
			if (sourceType == "ldap") {
				std::shared_ptr<object_list> l = ldap_get(ldap, type);
				if (l)
					add(type, l);
				else
					std::cout << type << " returned nullptr" << std::endl;
			}
		}
	} else {
		std::cout << "can't connect to ldap" << std::endl;
	}
	preload();
}


void data_server::preload() {
	data_cache_vector caches = json_data_file::json_to_ldap_cache_requests(
			config_file::instance().get("user-caches"));

//	for (const auto &cache : caches) {
//
//	}

}


/**
 * get all objects of type
 *
 * @param type the type of objects to return
 * @param query the query used to load the data
 * @return
 */
std::shared_ptr<object_list> data_server::get_by_type(const std::string &type) const {

	auto stuff = static_data.find(type);
	if (stuff != static_data.end())
		return stuff->second;

	stuff = dynamic_data.find(type);
	if (stuff != dynamic_data.end())
		return stuff->second;

	return std::make_shared<object_list>();
}

std::shared_ptr<object_list> data_server::get_static_by_type(const std::string &type) {
	auto stuff = static_data.find(type);
	if (stuff != static_data.end())
		return stuff->second;

	return std::make_shared<object_list>();
}


void data_server::add_dynamic(const std::string &type, std::shared_ptr<object_list> list) {
	auto type_data = dynamic_data.find(type);
	if (type_data == dynamic_data.end())
		dynamic_data.emplace(std::make_pair(type, list));
	else
		*type_data->second += *list;

}


void data_server::add_static(const std::string &type, std::shared_ptr<object_list> list) {
	static_data.emplace(std::make_pair(type, list));
}

void data_server::add(const std::string &type, base_object &&object) {
	auto list = get_by_type(type);
	if (list)
		list->add_object(object.get_uid(true), std::move(object));
	else {
		auto newList = std::make_shared<object_list>();
		newList->add_object(object.get_uid(true), std::move(object));
		add(type, newList);
	}
}



void data_server::add(const std::string &type, std::shared_ptr<object_list> list) {
	if (std::find(dynamic_types.begin(), dynamic_types.end(), type) != dynamic_types.end())
		add_dynamic(type, list);
	else
		add_static(type, list);
}

std::shared_ptr<base_object>
data_server::find_object_by_attribute(const std::string &type, const std::string &attrib, const std::string &value) {
	auto list = get_by_type(type);
	return list->get_object_for_attribute(attrib, value);
}




