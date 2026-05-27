#pragma once

namespace cvm {

struct RuntimeOptions {
    bool debug{false};
    bool compile_only{false};
    bool quiet{false};
};

}  // namespace cvm
