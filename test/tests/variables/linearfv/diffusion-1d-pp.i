[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 50
  []
[]

[Problem]
  linear_sys_names = 'u_sys'
[]

[Variables]
  [u]
    type = MooseLinearVariableFVReal
    solver_sys = 'u_sys'
    initial_condition = 1.0
  []
[]

[LinearFVKernels]
  [diffusion]
    type = LinearFVDiffusion
    variable = u
    diffusion_coeff = coeff_func
  []
  [source]
    type = LinearFVSource
    variable = u
    source_density = source_func
  []
[]

[LinearFVBCs]
  [dir]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = u
    boundary = "left right"
    functor = analytic_solution
  []
[]

[Functions]
  [coeff_func]
    type = ParsedFunction
    expression = '0.5*x'
  []
  [source_func]
    type = ParsedFunction
    expression = '2*x'
  []
  [analytic_solution]
    type = ParsedFunction
    expression = '1-x*x'
  []
[]

[Postprocessors]
  [average]
    type = ElementAverageValue
    variable = u
    execute_on = TIMESTEP_END
    outputs = csv
  []
  [min]
    type = ElementExtremeValue
    variable = u
    value_type = min
    execute_on = TIMESTEP_END
    outputs = csv
  []
  [max]
    type = ElementExtremeValue
    variable = u
    value_type = max
    execute_on = TIMESTEP_END
    outputs = csv
  []
  [num_dofs]
    type = NumDOFs
    execute_on = TIMESTEP_END
    outputs = csv
  []
  [elem_value]
    type = ElementalVariableValue
    variable = u
    elementid = 10
    execute_on = TIMESTEP_END
    outputs = csv
  []
  [point_value]
    type = PointValue
    variable = u
    point = '0.33333 0 0'
    execute_on = TIMESTEP_END
    outputs = csv
  []
[]

[VectorPostprocessors]
  [line-sample]
    type = LineValueSampler
    variable = u
    start_point = '0.13333 0 0'
    end_point = '0.766666 0 0'
    num_points = 9
    sort_by = x
    execute_on = TIMESTEP_END
    outputs = vpp_csv
  []
[]

[Executioner]
  type = Steady
  system_names = u_sys
  l_tol = 1e-10
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  [csv]
    type = CSV
    execute_on = TIMESTEP_END
  []
  [vpp_csv]
    type = CSV
    execute_on = TIMESTEP_END
  []
[]
