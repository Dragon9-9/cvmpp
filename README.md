# CVM++

C++17 scripting language with a bytecode compiler and stack VM.

```
.cvm → Lexer → Parser → Compiler → VM → output
```

## Build & run

```bash
make
./build/cvmpp examples/hello.cvm
./build/cvmpp              # REPL
./build/cvmpp -d script.cvm  # debug: tokens, AST, bytecode
make verify
```

## Language

Variables (`let`), functions (`fn` / `return`), `if` / `while`, `print`, `input`, operators `+ - * / == != < > <= >=`.

```cvm
fn factorial(n) {
  if (n <= 1) { return 1; }
  return n * factorial(n - 1);
}
print factorial(5);
```

## Docs

- [Project & build guide](docs/pdf/CVM++_01_Project_and_Build_Guide.pdf)
- [Architecture & workflow](docs/pdf/CVM++_02_Architecture_and_Workflow_Guide.pdf)

## License

MIT — see [LICENSE](LICENSE).
