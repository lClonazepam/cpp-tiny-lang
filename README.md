# cpp-tiny-lang

A complete front-to-back mini language:

```
source -> lexer -> Pratt-ish precedence parser -> AST
      -> bytecode compiler (jumps patched) -> stack VM
```

Language surface:

```
fn fib(n) {
  if (n < 2) { return n; }
  return fib(n - 1) + fib(n - 2);
}
fn main() {
  let i = 0;
  while (i < 10) {
    print fib(i);
    i = i + 1;
  }
  return 0;
}
```

Opcodes: `Const/Load/Store`, arithmetic + compare, `Jmp/Jz`, `Call/Ret`, `Print/Pop`.

Call frames live on a `frames` stack; locals/globals are a name table (simple, not a real activation record — fine for a teaching VM, and the next step is sliding a `bp` over a value stack).

## Build

```bash
cmake -S . -B build && cmake --build build -j
./build/tiny                  # built-in fib demo
./build/tiny examples/fib.tiny
```
