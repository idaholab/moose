# AGENTS.md

This file provides guidance to AI coding agents (e.g., GitHub Copilot, ChatGPT, Claude Code) when working with code in this repository.

## Mission

Extend the MOOSE framework with automatic weak form generation from energy functionals. Users describe physics through an energy density `f(u, \nabla u, ...)`, and the tooling derives the Euler–Lagrange equations, residuals, and Jacobians, including variable splitting for higher-order PDEs.

## Core Pieces to Know

- `framework/src/actions/AutomaticWeakFormAction.C`: orchestrates parsing energy expressions, builds kernels, and manages split-variable bookkeeping.
- `framework/include|src/utils/automatic_weak_form/`: symbolic AST (`MooseAST`), parsers, transformers, `WeakFormGenerator`, and simplification utilities.
- `framework/src/kernels/Variational*.C`: runtime kernels that evaluate residual/Jacobian contributions created from the generated weak forms.
- `unit/src/WeakFormDerivationTest.C` and `test/tests/variational`: current coverage for symbolic manipulation, residual assembly, and regression problems.

## Build & Test

```bash
cd test
make -j$(nproc)
./run_tests -i tests/variational
```

Unit tests under `unit/` use `gtest`; run them with the standard MOOSE testing harness.

## Current Gaps to Keep in Mind

- Higher-order derivative support and variable splitting are only partially implemented in `WeakFormGenerator` and `VariationalKernelBase`.
- The symbolic differentiator currently covers a narrow set of functions; new energy expressions may need additional derivative rules.
- Many unit tests assert exact string representations. Prefer semantic comparisons (e.g., structural equality, numeric spot-checks) when improving the suite.
- Runtime evaluation (`VariationalKernelBase::evaluateAtQP`) only handles a subset of AST node types; extending the algebra often requires augmenting this dispatch.

## Near-Term Roadmap (Automatic Splitting)

Goal: make automatic variable splitting usable for higher-order PDEs without relying on AuxVariables (all splits become regular `Variable`s so Newton solves cover the full system).

### 1. Flesh out the split analysis layer
- `framework/include/utils/automatic_weak_form/VariableSplitting.h`
  * Expand `VariableSplittingAnalyzer::analyzeNode` to track the maximum derivative order per original variable, explicitly handling nested gradients/divergence/laplacian/curl and cross-variable interactions.
  * Implement `generateSplitVariables` to build `SplitVariable` objects (names, shapes, defining expressions) for every derivative order beyond the FE capability; ensure naming is deterministic (`u_d2`, `u_d3`, etc.).
  * Implement `generateConstraintEquations` to return `NodePtr`s representing the residuals that enforce relationships between primary and split variables (e.g., `grad(u) - q = 0`).
  * Wire up `transformExpression` so it replaces high-order derivatives with split-variable references.
- Add focused unit tests in `unit/src/VariableSplittingAnalyzerTest.C` (new file) covering simple expressions like `laplacian(u)`, `div(grad(grad(u)))`, and mixed-variable cases; assert both the split map and the transformed AST strings.

### 2. Implement splitting strategies
- `framework/include/utils/automatic_weak_form/VariableSplitting.h` / `VariableSplitting.C`
  * Fill in `HigherOrderSplittingStrategy::createRecursive`, `createDirect`, and `createMixed` so they produce `SplitPlan`s with consistent dependency graphs when supplied with analyzer output.
  * Ensure `SplitPlan::dependencies` reference the actual variable names generated in step 1, not placeholder strings.
  * Update `optimizeBandwidth` to recompute `plan.bandwidth`/`total_dofs` based on the real split variables.
  * Extend the accompanying unit tests to cover the different strategies (e.g., `SplitPlan` for a 4th-order energy with recursive/direct/mixed options).

### 3. Connect splitting into the action
- `framework/src/actions/AutomaticWeakFormAction.C`
  * After parsing the energy, invoke the analyzer; when any primary variable exceeds the allowed order, insert new regular `addVariable` calls for the split variables (not AuxVariables).
  * Generate the constraint kernels using the new `SplitVariable.constraint_residual` expressions; hook them in via `addKernel` calls so they participate in the Newton solve.
  * Replace manual split bookkeeping in `_split_variables` with outputs from the analyzer/strategy.
  * Add logging/debug output summarizing the generated splits.

### 4. Validate end-to-end splitting
- Create a dedicated integration test (e.g., `test/tests/variational/fourth_order_auto_split.i`) that exercises the auto-splitting path and compares residual/Jacobian results to the existing hand-crafted split configuration.
- Ensure we cover at least one recursive and one direct strategy scenario; extend documentation (`doc/content`) to explain how the automatic splits are defined and how to inspect them.

Throughout the implementation, remember:
- Split variables must be standard `Variable`s (not AuxVariables) so the global Jacobian remains fully coupled for Newton.
- Preserve deterministic naming/ordering so caching, tape construction, and downstream kernels remain stable.
- Keep unit tests small and structural first; only add full physics regressions after the analysis layer is trustworthy.

## Longer-Term Roadmap

Once automatic splitting is reliable, focus on these broader improvements:

1. **Runtime parity & performance**
   - Expand `ExpressionEvaluator` and the evaluation tape to cover all node types supported by the differentiator (vectors, tensors, cross/curl, custom functions).
   - Cache symbolic coefficients (C⁰/C¹/C²/…) per element and use tapes to evaluate residual/Jacobian contributions without re-running differentiation inside quadrature loops.
   - Profile the new path versus the legacy implementation and optimize hotspots (shared subexpressions, shape checks, memory reuse).

2. **Robustness & diagnostics**
   - Flesh out `VariationalProblemAnalyzer` to perform real well-posedness/coercivity checks and emit actionable warnings.
   - Improve error messages when unsupported AST constructs or derivative orders appear (point back to the offending energy term).
   - Add richer debug output: variable split summaries, tape dumps, residual/Jacobian previews.

3. **User-facing features**
   - Document automatic weak-form capabilities and limitations in the MOOSE manuals (action pages, tutorials, example input files).
   - Provide sample problems (e.g., Cahn–Hilliard, gradient flows, mixed elasticity) that demonstrate automatic splitting and tape-enabled evaluation.
   - Expose configuration options (strategy selection, split naming) via input parameters with sensible defaults.

4. **Extensibility**
   - Make it straightforward to register new functions/operators: centralize derivative rules, evaluation rules, and simplification hooks.
   - Establish a regression suite of manufactured solutions (residual/Jacobian checkers) to guard against future changes.

## Contribution Expectations

1. Keep the AST transformations deterministic so downstream actions and kernels remain reproducible.
2. When adding new syntax, update the parser, differentiation visitor, simplifier, documentation, and tests together.
3. Document limitations and edge cases explicitly—automatic weak form generation is sensitive to hidden assumptions.
4. Prefer incremental tests that exercise the full pipeline: expression → differentiation → weak form → residual/Jacobian evaluation.

## Troubleshooting Tips

- Enable `[AutomaticWeakForm] verbose = true` in input files to inspect transformed expressions and generated kernels.
- Compare generated residuals against hand-derived ones on small manufactured problems to validate new features.
- If differentiation or evaluation fails on new operations, start by adding the missing node handling in `DifferentiationVisitor` and `evaluateAtQP`.

Happy hacking!
