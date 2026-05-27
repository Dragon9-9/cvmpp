#include "cvm++/value.hpp"

#include <stdexcept>

namespace cvm {

bool is_int(const VmValue& value) { return std::holds_alternative<std::int64_t>(value); }

bool is_bool(const VmValue& value) { return std::holds_alternative<bool>(value); }

std::int64_t as_int(const VmValue& value) {
    if (!is_int(value)) {
        throw std::runtime_error("expected integer value");
    }
    return std::get<std::int64_t>(value);
}

bool as_bool(const VmValue& value) {
    if (!is_bool(value)) {
        throw std::runtime_error("expected boolean value");
    }
    return std::get<bool>(value);
}

std::string value_to_string(const VmValue& value) {
    if (is_int(value)) {
        return std::to_string(as_int(value));
    }
    return as_bool(value) ? "true" : "false";
}

std::string value_type_name(const VmValue& value) {
    return is_int(value) ? "integer" : "boolean";
}

}  // namespace cvm
