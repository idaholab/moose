[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
  xmax = 1
  ymax = 1
[]

[Variables]
  [u]
  []
[]

# Sets solution invalid using the SolutionInvalidInterface, as diffusivity exceeds the set threshold.
[Materials]
  [filter]
    type = NonsafeMaterial
    diffusivity = 0.5
    threshold = 0.3
  []
[]

[Kernels]
  [diffusion]
    type = MatDiffusion
    variable = u
    diffusivity = diffusivity
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 1
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 0
  []
[]

[Problem]
  type = FEProblem
  allow_invalid_solution = false
  immediately_print_invalid_solution = false
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu superlu_dist'
[]

[Reporters/solution_invalidity]
  type = SolutionInvalidityReporter
  execute_on = FINAL
[]

[Outputs]
  file_base = 'solution_invalid'
  [out]
    type = JSON
    execute_on = 'FINAL'
  []
[]
