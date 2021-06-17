# HMG

## Overview

HMG stands for High-performance (Hybrid) MultiGrid method.
[HMG](https://www.mcs.anl.gov/petsc/petsc-current/docs/manualpages/PC/PCHMG.html)'s
essential idea is to separate a multigrid method into two steps;
matrix coarsening and level solvers. The main motivation of HMG in
the first place is to use
[HYPRE](https://computing.llnl.gov/projects/hypre-scalable-linear-solvers-multigrid-methods)
to do matrix coarsening and generate interpolations
and use [PETSc](https://www.mcs.anl.gov/petsc/) preconditioners/solvers as level solvers.
However, the code is more general, and it can use other
codes such as [GAMG](https://www.mcs.anl.gov/petsc/petsc-current/docs/manualpages/PC/PCGAMG.html)
or a user code to generate interpolations.


## Example 1

```
[Executioner]
  type = Steady

  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type'
  petsc_options_value = 'hmg'
[]
```

This configuration uses HYPRE to generate interpolations
and uses SOR preconditioners from PETSc as level solvers.

## Example 2

```
[Executioner]
  type = Steady

  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_hmg_use_subspace_coarsening'
  petsc_options_value = 'hmg true'
  petsc_options = '-snes_view'
[]
```

If there are multiple nonlinear variables, this configuration will reuse interpolations
generated for the first nonlinear variable for all other variables. This will significantly
speedup preconditioner setup. A complete toy example is [test/tests/preconditioners/hmg/diffusion_hmg.i].
We also demonstrate this capability for
realistic neutron transport calculations in the following paper:

```tex
@article{kong2020highly,
  title={{A Highly Parallel Multilevel Newton--Krylov--Schwarz Method with Subspace-Based Coarsening and Partition-Based Balancing for the Multigroup Neutron Transport Equation on Three-Dimensional Unstructured Meshes}},
  author={Kong, Fande and Wang, Yaqi and Gaston, Derek R and Permann, Cody J and Slaughter, Andrew E and Lindsay, Alexander D and DeHart, Mark D and Martineau, Richard C},
  journal={SIAM Journal on Scientific Computing},
  volume={42},
  number={5},
  pages={C193--C220},
  year={2020},
  publisher={SIAM}
}
```
