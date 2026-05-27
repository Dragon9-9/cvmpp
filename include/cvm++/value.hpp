#pragma once

#include <cstdint>
#include <string>
#include <variant>

namespace cvm {

using VmValue = std::variant<std::int64_t, bool>;

bool is_int(const VmValue& value);
bool is_bool(const VmValue& value);

std::int64_t as_int(const VmValue& value);
bool as_bool(const VmValue& value);

std::string value_to_string(const VmValue& value);
std::string value_type_name(const VmValue& value);

}  // namespace cvm
