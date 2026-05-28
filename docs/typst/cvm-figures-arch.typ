// Compact, non-floating diagrams for the Architecture guide only
#import "cvm-theme.typ": tbl-tight, diagram
#import "cvm-figures.typ": ast-tree

#let arch-bytecode-layout() = diagram(
  [`code[]` layout: `JUMP` → optional `fn` bodies → main `statements[]` → `HALT`; metadata in `names[]` and `functions[]`.],
  [
    #tbl-tight(
      ("Region", "Contents"),
      (
        [Byte 0], [`JUMP` to main entry],
        [Low offsets], [`fn` bodies (if any)],
        [Main], [`Program::statements`],
        [End], [`HALT`],
      ),
    )
  ],
)

#let arch-ast-hello() = diagram(
  [`hello.cvm` AST (`cvmpp -d`): `functions[]` empty; four `statements[]` nodes.],
  [
    #ast-tree(
      "Program\n|-- Let (x) → 42\n|-- Let (flag) → true\n|-- Print (x)\n+-- If (x < 100) → print (x + 8)",
    )
  ],
)

#let arch-ast-functions() = diagram(
  [`functions.cvm` AST: `fn` in `functions[]`; `let` and `print` in `statements[]`.],
  [
    #ast-tree(
      "Program\n+-- Fn factorial(n)\n|-- Let (x) → Call factorial(5)\n+-- Print (x)",
    )
  ],
)

#let arch-if-jump() = diagram(
  [`if (x < 100)`: compile condition; `JUMP_IF_FALSE` skips then-branch (patched to 58 in `hello.cvm`).],
  [
    #tbl-tight(
      ("Offsets", "Role"),
      (
        [26–38], [Evaluate `x < 100`],
        [39], [`JUMP_IF_FALSE` — if false, jump to 58],
        [44–57], [Then: `print x + 8`],
        [58], [`HALT`],
      ),
    )
  ],
)

#let arch-functions-bytecode() = diagram(
  [`functions.cvm` (73 bytes): byte 0 jumps to main; `factorial` is at low offsets; `CALL` enters function code.],
  [
    #tbl-tight(
      ("Offsets", "Region"),
      (
        [0–4], [`JUMP` to main],
        [5–51], [`factorial` body],
        [52–71], [main: `CALL`, store, `PRINT`],
        [72], [`HALT`],
      ),
    )
  ],
)

#let arch-recursion-ladder() = diagram(
  [`factorial(5)` call ladder — descend on `CALL`, then unwind on `RETURN`.],
  [
    #tbl-tight(
      ("Phase", "Depth", "Frame (`slot 0 = n`)", "Event"),
      (
        [descend], [0], [`n=5`], [`CALL factorial(4)`],
        [descend], [1], [`n=4`], [`CALL factorial(3)`],
        [descend], [2], [`n=3`], [`CALL factorial(2)`],
        [descend], [3], [`n=2`], [`CALL factorial(1)`],
        [base], [4], [`n=1`], [`RETURN 1`],
        [unwind], [3], [`n=2`], [`RETURN 2*1 = 2`],
        [unwind], [2], [`n=3`], [`RETURN 3*2 = 6`],
        [unwind], [1], [`n=4`], [`RETURN 4*6 = 24`],
        [unwind], [0], [`n=5`], [`RETURN 5*24 = 120`],
      ),
    )
  ],
)
