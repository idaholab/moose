# Steady state Heat conduction in a 3D domain with two diffusivities
# The domain is -2 <= x <= 2, -2 <= y <= 2, and 0 <= z <=2
# The top-half of the domain (z > 1) has high diffusivity
# The bottom-half of the domain (z < 1) has low diffusivity
# This is used to generate the synthetic_data

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 3
    xmin = -2
    xmax = 2
    ymin = -2
    ymax = 2
    zmin = 0
    zmax = 2
    nx = 16
    ny = 16
    nz = 8
  []
[]

[Variables]
  [temperature]
  []
[]

[Kernels]
  [heat_conduction]
    type = MatDiffusion
    variable = temperature
    diffusivity = diffusivity
  []
  [heat_source]
    type = BodyForce
    function = '200*(sin(pi*z)+1.0)*(sin(pi*(x+2.0)/2.0)+1.0)'
    variable = temperature
  []
[]

[AuxVariables]
  [grad_Tx]
    order = CONSTANT
    family = MONOMIAL
  []
  [grad_Ty]
    order = CONSTANT
    family = MONOMIAL
  []
  [grad_Tz]
    order = CONSTANT
    family = MONOMIAL
  []
  [aux_diffusivity]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [aux_diffusivity]
    type = FunctionAux
    variable = aux_diffusivity
    function = diffusivity_function
    execute_on=TIMESTEP_END
  []
[]

[BCs]
  [bottom]
    type = DirichletBC
    variable = temperature
    boundary = back
    value = 0
  []
[]

[Functions]
  [diffusivity_function]
    # sandwitch-like material diffusivity
    type = ParsedFunction
    expression = 'if (z<1, 5, 10)'
  []
[]

[Materials]
  [mat]
    type = GenericFunctionMaterial
    prop_names = diffusivity
    prop_values = diffusivity_function
  []
[]

[VectorPostprocessors]
  [line_temperature]
    type = LineValueSampler
    variable = temperature
    start_point = '-2 -2 2'
    end_point = '2 2 2'
    num_points = 10
    sort_by = z
  []
[]


[Executioner]
  type = Steady
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  nl_forced_its = 1
  line_search = none
  nl_abs_tol = 1e-8
[]

[Outputs]
  file_base = 'forward'
  console = false
  exodus = true
  csv = true
[]
