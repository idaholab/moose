[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 3
  ny = 3
[]

[Variables]
  [u]
    type = MooseVariableFVReal
  []
[]

[FVKernels]
  [diff]
    type = FVDiffusion
    variable = u
    coeff = 1
  []
[]

[FVBCs]
  [left]
    type = FVNeumannBC
    variable = u
    boundary = left
    value = 18
  []
  [right]
    type = FVDirichletBC
    variable = u
    boundary = right
    value = 1
  []
[]

[Postprocessors]
  [flux_left]
    type = SideFVFluxBCIntegral
    boundary = left
    fvbcs = 'left'
  []
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  nl_abs_tol = 1e-9
  nl_rel_tol = 1e-9
  l_abs_tol = 1e-9
  l_tol = 1e-6
[]

[Outputs]
  csv = true
  execute_on = final
[]
