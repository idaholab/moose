[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 20
    xmin = 0
    xmax = 1
  []
[]

[Problem]
  linear_sys_names = 'u_sys'
[]

[Variables]
  [u]
    type = MooseLinearVariableFVReal
    solver_sys = 'u_sys'
    initial_condition = 0.0
  []
[]

[LinearFVKernels]
  [diffusion]
    type = LinearFVDiffusion
    variable = u
  []
[]

[LinearFVBCs]
  [left]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = u
    boundary = left
    functor = 0
  []
  # "Off" right BC: u=0, initially enabled; disabled by Control
  [right_off]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = u
    boundary = right
    functor = 0
    enable = true
  []
  # "On" right BC: u=1, initially disabled; enabled by Control
  [right_on]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = u
    boundary = right
    functor = 1
    enable = false
  []
[]

[Functions]
  [always_on]
    type = ParsedFunction
    expression = '1'
  []
[]

[Controls]
  [switch_right_bc]
    type = ConditionalFunctionEnableControl
    conditional_function = always_on
    enable_objects = 'LinearFVBCs::right_on'
    disable_objects = 'LinearFVBCs::right_off'
  []
[]

[Postprocessors]
  [side_avg_right]
    type = SideAverageValue
    variable = u
    boundary = right
  []
[]

[Executioner]
  type = Steady
  system_names = u_sys
  l_tol = 1e-10
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[Outputs]
  [out]
    type = CSV
    execute_on = 'TIMESTEP_END'
  []
[]
