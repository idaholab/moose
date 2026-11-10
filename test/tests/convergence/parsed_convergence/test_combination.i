# Combines two DefaultNonlinearConvergence objects, each with a different nl_abs_tol, into a
# single nonlinear_convergence via ParsedConvergence. Each object must check its own residual
# norm against its own tolerance rather than a tolerance retrieved from a shared PETSc SNES
# object, or the tolerance actually reported/used by one object would depend on which object's
# setup happened to run last.

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  []
[]

[Convergence]
  [loose_conv]
    type = DefaultNonlinearConvergence
    nl_abs_tol = 1e-3
    verbose = true
  []
  [tight_conv]
    type = DefaultNonlinearConvergence
    nl_abs_tol = 1e-10
    verbose = true
  []
  [combined]
    type = ParsedConvergence
    symbol_names = 'loose tight'
    symbol_values = 'loose_conv tight_conv'
    convergence_expression = 'loose & tight'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  nonlinear_convergence = combined
[]
