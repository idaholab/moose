!include pmg_diffusion.i

[Kernels]
  active = 'nonlinear rhs'
  [nonlinear]
    type = MFEMNLDiffusionKernel
    variable = concentration
    k_coefficient = concentration
    dk_du_coefficient = 1.0
  []
[]

[Solvers]
  [nonlinear]
    type = MFEMNewtonNonlinearSolver
  []
[]
