#include "system_properties.h"

#include <unistd.h>
#include <iostream>

void shim::add_system_properties_shimmed_symbols(std::vector<shim::shimmed_symbol> &list) {
    list.push_back({"__system_property_find", __system_property_find});
    list.push_back({"__system_property_get", __system_property_get});
}

const prop_info* shim::__system_property_find(const char *name) {
    std::cout << "__system_property_find stub called with name: " << name << std::endl;
    return 0;
}

int shim::__system_property_get(const char *name, char *value) {
    std::cout << "__system_property_get stub called with name: " << name << std::endl;
    value[0] = 0;
    return 0;
}
