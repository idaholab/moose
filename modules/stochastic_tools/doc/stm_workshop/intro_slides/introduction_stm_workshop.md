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

!style halign=center fontsize=150%
Perform UQ and sensitivity analysis with +distributed data+.

!row!
!col width=40%
!media dispx_center_inner_hist.png

!col width=60%
!media sobol_total.png
!row-end!

!---

!style halign=center fontsize=150%
Train meta-models to build fast-evaluating +surrogates+ of the high-fidelity multiphysics model.

!style halign=center fontsize=150%
Provide a +pluggable+ interface for these surrogates.

!media surrogate_viz.png

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
