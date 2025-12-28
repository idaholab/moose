# Differentiation Coverage Snapshot

Enumerates current `NodeType` values defined in `MooseAST` and whether `DifferentiationVisitor` implements a rule today.

| NodeType | Present Handler | Notes |
| --- | --- | --- |
| Constant | ✔️ | Returns zero differential. |
| Variable | ✔️ | Zeroth-order coefficient = 1 for matching variable name. |
| FieldVariable | ✔️ | Same as `Variable`. |
| TestFunction | ❌ | Should be treated as independent symbol; currently triggers unsupported branch. |
| ShapeFunction | ❌ | No handler; required once Jacobian/test substitution is symbolic. |
| Add | ✔️ | `handleAdd`. |
| Subtract | ✔️ | `handleSubtract`. |
| Multiply | ⚠️ | `handleMultiply` covers limited tensor combinations; lacks product rules for higher-order outputs. |
| Divide | ✔️ | `handleDivide`. |
| Power | ⚠️ | Only handles scalar exponents; missing general derivative for `u^v`. |
| Negate | ✔️ | Unary minus. |
| Gradient | ✔️ | Bumps derivative order by 1 irrespective of operand shape. |
| Divergence | ⚠️ | Decreases order assuming tensor-to-vector mapping; lacks validation of operand rank. |
| Laplacian | ✔️ | Adds two derivative orders; assumes scalar field. |
| Curl | ❌ | No implementation. |
| Dot | ⚠️ | Product rule assumes matching dependencies; needs tensor generalization. |
| Cross | ❌ | Not supported. |
| Contract | ⚠️ | Handles limited cases; does not propagate cross-terms. |
| Outer | ⚠️ | Missing full product rule for mixed dependencies. |
| Norm | ⚠️ | Only first-order chain rule; higher orders unhandled. |
| Normalize | ⚠️ | First-order only; ignores derivative of normalization factor for higher orders. |
| Trace | ✔️ | Applies `trace` to operand coefficient. |
| Determinant | ⚠️ | Handles only first-order using adjugate formula; needs higher orders. |
| Inverse | ⚠️ | Only first-order rule implemented. |
| Transpose | ✔️ | Applies transpose. |
| Symmetric | ✔️ | Applies `sym`. |
| Skew | ✔️ | Applies `skew`. |
| Deviatoric | ✔️ | Uses trace subtraction; assumes 3D. |
| Function | ⚠️ | Hard-coded derivatives for `W`, `log`, `exp`, `sin`, `cos`; all other functions unsupported. |
| TensorComponent | ❌ | No handler. |
| VectorComponent | ❌ | No handler. |
| VectorAssembly | ⚠️ | Builds component-wise differentials but allocates zero placeholders; needs cleanup.

Additional notes:
- Registry-based dispatch now ensures memoized reuse of repeated subtrees; the table highlights remaining semantic coverage gaps.
- Derivative order bookkeeping still assumes scalar base functions and does not enforce FE shape compatibility (scalar vs vector trial/test functions).
- Chain/product rules ignore derivative orders > 1 for mixed dependencies, so C²/C³ coefficients are incomplete even for supported node types.

This table should be updated once the registry-based differentiation refactor lands.
