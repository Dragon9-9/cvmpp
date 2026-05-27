#pragma once

#include "cvm++/source_loc.hpp"

#include <string>
#include <vector>

namespace cvm {

enum class Phase {
    Lexer,
    Parser,
    Compiler,
    Vm,
    Repl,
};

enum class Severity {
    Error,
    Warning,
    Note,
};

struct Diagnostic {
    Phase phase{Phase::Lexer};
    Severity severity{Severity::Error};
    std::string message;
    SourceLoc loc{};
    std::string hint;
};

class DiagnosticBag {
public:
    void push(Diagnostic d);
    bool has_errors() const;
    const std::vector<Diagnostic>& all() const { return items_; }
    void clear();

private:
    std::vector<Diagnostic> items_;
};

std::string phase_label(Phase p);
std::string severity_label(Severity s);

}  // namespace cvm
