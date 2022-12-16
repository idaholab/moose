[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
  []
[]

[Variables/u]
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
  [src]
    type = BodyForce
    variable = u
    function = src_func
  []
[]

[BCs]
  [dirichlet]
    type = DirichletBC
    variable = u
    boundary = 'bottom left'
    value = 0
  []
[]

[Functions]
  [src_func]
    type = ParameterMeshFunction
    exodus_mesh = parameter_mesh_in.e
    parameter_name = src_rep/vals
  []
[]

[Reporters]
  [src_rep]
    type = ConstantReporter
    real_vector_names = 'vals'
    real_vector_values = '1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0 1.0'
  []
  [measure_data]
    type = OptimizationData
    variable = u
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[AuxVariables]
  [source]
  []
[]

[AuxKernels]
  [source_aux]
    type = FunctionAux
    variable = source
    function = src_func
  []
[]

[Outputs]
  exodus = true
  console = false
  execute_on = timestep_end
[]
