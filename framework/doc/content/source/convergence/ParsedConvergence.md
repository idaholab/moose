# ParsedConvergence

This [Convergence](Convergence/index.md) allows the user to specify arbitrary expressions
for convergence and divergence criteria. These expressions
([!param](/Convergence/ParsedConvergence/convergence_expression) and [!param](/Convergence/ParsedConvergence/divergence_expression))
may contain any of the following:

- `Convergence` objects
- [Functions](Functions/index.md)
- [Post-processors](Postprocessors/index.md)
- Constant values

The expressions are parsed using the [Function Parser syntax](http://warp.povusers.org/FunctionParser/fparser.html#functionsyntax).
The full library of mathematical operators is valid in the parsed
expression, but for convenience, we list some of the logical and comparison operators here:

| Syntax | Description |
| :- | :- |
| `()` | Parentheses for order of operations |
| `!A` | *NOT* `A` |
| `A & B` | `A` *AND* `B` |
| `A` I `B` | `A` *OR* `B` |
| `A = B` | `A` *EQUALS* `B` |
| `A != B` | `A` *DOES NOT EQUAL* `B` |
| `A >= B` | `A` *GREATER THAN OR EQUAL TO* `B` |

The expressions must evaluate to either 1 or 0, which correspond to `true` or `false`,
respectively; if the expression returns another value, an error results. Note
the following rules for the `Convergence` object values:

- For the convergence expression, `Convergence` objects evaluate to `true` if they
  are `CONVERGED` and `false` otherwise (`ITERATING` or `DIVERGED`).
- For the divergence expression, `Convergence` objects evaluate to `true` if they
  are `DIVERGED` and `false` otherwise (`ITERATING` or `CONVERGED`).

The divergence expression is optional. If omitted, divergence occurs if any of
the supplied `Convergence` objects return `DIVERGED`, e.g.,

```
divergence_expression = 'conv1 | conv2 | conv3'
```

if [!param](/Convergence/ParsedConvergence/symbol_values) contains `conv1`, `conv2`, and `conv3`.

!syntax parameters /Convergence/ParsedConvergence

!syntax inputs /Convergence/ParsedConvergence

!syntax children /Convergence/ParsedConvergence
