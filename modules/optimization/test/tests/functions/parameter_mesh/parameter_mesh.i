[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
  []
[]

[Functions]
  [parameter_mesh]
    type = ParameterMeshFunction
    exodus_mesh = create_mesh_out.e
    parameter_name = param_vec/params
  []
[]

[VectorPostprocessors]
  [param_vec]
    type = CSVReader
    csv_file = create_mesh_out_param_vec_0001.csv
  []
[]

[AuxVariables]
  [parameter]
    family = MONOMIAL
    order = CONSTANT
  []
  [grad_parameter]
    family = MONOMIAL_VEC
    order = CONSTANT
  []
  [parameter_gradient]
    components = 9
  []
[]

[AuxKernels]
  [parameter_aux]
    type = FunctionAux
    function = parameter_mesh
    variable = parameter
  []
  [grad_parameter_aux]
    type = FunctorElementalGradientAux
    functor = parameter_mesh
    variable = grad_parameter
  []
  [parameter_gradient_aux]
    type = OptimizationFunctionAuxTest
    function = parameter_mesh
    variable = parameter_gradient
  []
[]

[Outputs]
  exodus = true
  execute_on = timestep_end
[]

[Executioner]
  type = Steady
[]

[Problem]
  solve = false
[]
