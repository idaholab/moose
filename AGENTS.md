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
