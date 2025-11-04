aux_execute_on = 'INITIAL NONLINEAR_CONVERGENCE TIMESTEP_END'
pp_execute_on = 'INITIAL NONLINEAR_CONVERGENCE TIMESTEP_END'
output_execute_on = 'INITIAL NONLINEAR TIMESTEP_END'

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  xmax = 2
  ymax = 2
[]

[Variables]
  [sol]
  []
[]

[AuxVariables]
  [aux]
  []
[]

[AuxKernels]
  [aux_kernel]
    type = ParsedAux
    variable = aux
    functor_names = 'sol'
    functor_symbols = 'u'
    expression = 'u'
    execute_on = ${aux_execute_on}
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = sol
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = sol
    boundary = left
    value = 0
  []
  [right]
    type = DirichletBC
    variable = sol
    boundary = right
    value = 1
  []
[]

[Problem]
  previous_nl_solution_required = true
[]

[Executioner]
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'hypre'
  nl_rel_tol = 0
  nl_abs_tol = 1e-8
[]

[Outputs]
  csv = true
  execute_on = ${output_execute_on}
[]
