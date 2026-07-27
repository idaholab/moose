# NEML2 Constitutive Model Library

## Overview id=overview

[NEML2](https://applied-material-modeling.github.io/neml2/) (the New Engineering Material model
Library, version 2) is an external, open-source library of constitutive models that the Solid
Mechanics module can evaluate.  It provides a broad catalogue of elasticity, plasticity,
viscoplasticity, damage, and crystal-plasticity models, evaluated in batched (and optionally
GPU-accelerated) form, and is the recommended home for new constitutive-model development.

## Using NEML2 id=using

- **Install.** NEML2 support is optional and enabled at build time; see
  [Installing NEML2](install_neml2.md optional=True).
- **Model catalogue.** The available models and their parameters are documented in the
  [NEML2 solid mechanics documentation](https://applied-material-modeling.github.io/neml2/stable/content/modules/solid_mechanics/index.html).
- **Coupling.** A NEML2 model is loaded and evaluated through the module's NEML2 interface, which
  gathers the required inputs (strain, temperature, history) from the simulation, evaluates the model,
  and returns the stress and its tangent to the [stress-divergence kernels](BalanceOfLinearMomentum.md).

Because NEML2 models supply the stress and algorithmic tangent directly, they slot into the same
constitutive interface as the [built-in stress calculators](CustomConstitutiveModels.md).
