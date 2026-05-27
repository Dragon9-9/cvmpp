#include "cvm++/source_loc.hpp"

#include <sstream>

namespace cvm {

std::string SourceLoc::to_string() const {
    std::ostringstream oss;
    oss << line << ':' << column;
    return oss.str();
}

}  // namespace cvm
