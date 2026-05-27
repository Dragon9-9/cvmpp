#include "cvm++/diagnostic.hpp"

namespace cvm {

void DiagnosticBag::push(Diagnostic d) {
    items_.push_back(std::move(d));
}

bool DiagnosticBag::has_errors() const {
    for (const auto& d : items_) {
        if (d.severity == Severity::Error) {
            return true;
        }
    }
    return false;
}

void DiagnosticBag::clear() {
    items_.clear();
}

std::string phase_label(Phase p) {
    switch (p) {
        case Phase::Lexer:
            return "LEXER";
        case Phase::Parser:
            return "PARSER";
        case Phase::Compiler:
            return "COMPILER";
        case Phase::Vm:
            return "VM";
        case Phase::Repl:
            return "REPL";
    }
    return "UNKNOWN";
}

std::string severity_label(Severity s) {
    switch (s) {
        case Severity::Error:
            return "error";
        case Severity::Warning:
            return "warning";
        case Severity::Note:
            return "note";
    }
    return "unknown";
}

}  // namespace cvm
