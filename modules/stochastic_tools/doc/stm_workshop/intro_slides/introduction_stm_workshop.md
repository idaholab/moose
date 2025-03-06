# Introduction

!---

!style halign=center fontsize=150%
*The STM is an open-source MOOSE module available to all MOOSE-based applications.*

- Provide a +MOOSE-like interface+ for performing stochastic analysis on MOOSE-based models.
- Sample parameters, run applications, and gather data that is both +efficient+ (memory and runtime) and +scalable+.

!row!
!col width=50%
!media normal_viz.png style=width:75%;margin-left:auto;margin-right:auto;display:block

!col width=50%
!media batch_viz.png
!row-end!

!---

!style halign=center fontsize=120%
Perform UQ and sensitivity analysis with +distributed data+ and leveraging +advanced variance reduction+ methods.

!media sobol_total.png style=width:50%;margin-left:auto;margin-right:auto;display:block

!media case1_inference_results.png style=width:50%

!media Parallel_Subset_Simulation_Sampler.svg style=width:35%

!---

!style halign=center fontsize=120%
Train meta-models to build fast-evaluating +surrogates+ of the high-fidelity multiphysics model and provide a +pluggable+ interface for these surrogates.

- Harness machine learning capabilities through the C++ front end of PyTorch, i.e. +Libtorch+.
- Use +active learning+ models for building surrogates.

!media surrogate_viz.png style=width:60%

!media gp_variants.png style=width:40%

!media active_learning.png style=width:75%;display:block;margin-left:auto;margin-right:auto;

!---

# Focus of this Training

Part 1: Introduction to systems and basic parameter studies

- `Distributions` and `Samplers`
- Model execution via `MultiApps`
- Parameter study statistics and sensitivity analysis
- Introduction to surrogate system

Part 2^*^: Bayesian (inverse) uncertainty quantification

- Model parameter uncertainties
- Model parameter + noise uncertainties
- Model parameter + noise + model inadequacy uncertainties

Part 3^*^: Active learning

- Training a Gaussian process surrogate with Monte Carlo sampling
- Use of an acquisition function
- Solving optimization problems with Bayesian optimization

[!style fontsize=80%](* Not included in these slides)

!---

# Workflow

!media stm_flow.png

!---

# Stochastic Tools Syntax

- Since STM runs as a wrapper, it does not have typical MOOSE physics objects
- The use of this syntax produces a minimal FEM problem required by the MOOSE system

```
[StochasticTools]
[]
```

- With this block, the following typical objects are no longer required:

  - Mesh
  - Variables
  - Kernels
  - Executioner

!--

# Example Problem Statement

### Governing Equation

!equation
\frac{\partial T}{\partial t} - \nabla\cdot D \nabla T = q,\, x\in [0, 1],\, y\in [0, 1]

!equation
T(t, x=0, y) = T_0 ,\quad
\left. -D\frac{\partial T}{\partial x}\right|_{x=1} = q_0,\quad
\left. D\frac{\partial T}{\partial y}\right|_{y=0, y=1} = 0,\quad
T(t=0, x, y) = 300

!---

### Physics Input

!row!

!col! width=50%

!style fontsize=60%
!listing examples/workshop/diffusion.i

!col-end!

!col! width=50%

!style halign=center style=width:80%;margin-left:auto;margin-right:auto;display:block
```bash
moose-opt -i diffusion.i
```

!style fontsize=60% halign=center style=width:80%;margin-left:auto;margin-right:auto;display:block
```
Postprocessor Values:
+----------------+----------------+----------------+
| time           | T_avg          | q_left         |
+----------------+----------------+----------------+
|   0.000000e+00 |   3.000000e+02 |   1.026734e-13 |
|   2.500000e-01 |   2.945503e+02 |   1.691165e+01 |
|   5.000000e-01 |   2.903864e+02 |   1.162035e+01 |
|   7.500000e-01 |   2.876841e+02 |   5.770252e+00 |
|   1.000000e+00 |   2.859939e+02 |   1.733798e+00 |
+----------------+----------------+----------------+
```

!col-end!

!row-end!

!--

### Parameters and Quantities of Interest (QoIs)

Four uncertain parameters:

!table
| Parameter | Symbol | Syntax | Distribution |
| :- | - | :- | - |
| Diffusivity | $D$ | `Materials/constant/prop_values` | Uniform(0.5, 2.5) |
| Source | $q$ | `Kernel/source/function` | Normal(100, 25) |
| Temperature | $T_0$ | `BCs/left/value` | Normal(300, 45) |
| Flux | $q_0$ | `BCs/right/value` | Weibull(1, 20, -110) |

Two quantities of interest:

!table
| QoI | Symbol | Syntax |
| :- | - | :- |
| Average Temperature | $T_{\mathrm{avg}}$ | `Postprocessors/T_avg` |
| Left Heat Flux | $q_{\mathrm{left}}$ | `Postprocessors/q_left` |
