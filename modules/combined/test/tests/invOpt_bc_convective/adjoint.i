[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 20
    xmax = 1
    ymax = 2
  []
[]

[AuxVariables]
  [temperature_forward]
  []
  [T2]
  []
[]

[AuxKernels]
  [TT]
    type = ParsedAux
    args = 'temperature temperature_forward'
    variable = T2
    function = 'temperature*(100-temperature_forward)'
  []
[]

[Variables]
  [temperature]
  []
[]

[Kernels]
  [heat_conduction]
    type = ADHeatConduction
    variable = temperature
  []
[]

[DiracKernels]
  [pt]
    type = ReporterPointSource
    variable = temperature
    x_coord_name = misfit/measurement_xcoord
    y_coord_name = misfit/measurement_ycoord
    z_coord_name = misfit/measurement_zcoord
    value_name = misfit/misfit_values
  []
[]

[Reporters]
  [misfit]
    type = OptimizationData
  []
  [params]
    type = ConstantReporter
    real_vector_names = 'vals'
    real_vector_values = '0' # Dummy value
  []
[]

[BCs]
  [left]
    type = ConvectiveFluxFunction
    variable = temperature
    boundary = 'left'
    T_infinity = 0.0
    coefficient = function1
  []
  [right]
    type = NeumannBC
    variable = temperature
    boundary = right
    value = 0
  []
  [bottom]
    type = DirichletBC
    variable = temperature
    boundary = bottom
    value = 0
  []
  [top]
    type = DirichletBC
    variable = temperature
    boundary = top
    value = 0
  []
[]

[Materials]
  [steel]
    type = ADGenericConstantMaterial
    prop_names = thermal_conductivity
    prop_values = 5
  []
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  nl_abs_tol = 1e-6
  nl_rel_tol = 1e-8
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[Functions]
  [function1]
    type = ParsedOptimizationFunction
    expression = 'a'
    param_symbol_names = 'a'
    param_vector_name = 'params/vals'
  []
[]

[VectorPostprocessors]
  [adjoint_pt]
    type = SideOptimizationNeumannFunctionInnerProduct
    variable = T2
    function = function1
    boundary = left
  []
[]

[Outputs]
  console = false
  exodus = false
  file_base = 'adjoint'
[]
