[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 10
    xmax = 0.5
    xmin = -0.5
  []
  [left_line]
    type = SubdomainBoundingBoxGenerator
    input = gmg
    bottom_left = '-0.5 0 0'
    top_right = '0 0 0'
    block_id = 1
    block_name = 'left_line'
    location = INSIDE
  []
  [right_line]
    type = SubdomainBoundingBoxGenerator
    input = left_line
    bottom_left = '0 0 0'
    top_right = '0.5 0 0'
    block_id = 2
    block_name = 'right_line'
    location = INSIDE
  []
[]

[Variables]
  [temperature]
  []
[]

[Kernels]
  [time_derivative]
    # type = HeatConductionTimeDerivative
    type = TrussHeatConductionTimeDerivative
    variable = temperature
    area = area
  []
  [heat_conduction]
    # type = HeatConduction
    type = TrussHeatConduction
    variable = temperature
    area = area
  []
[]

[AuxVariables]
  [area]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [area]
    type = ConstantAux
    variable = area
    value = 0.1
    execute_on = 'initial timestep_begin'
  []
[]

[Materials]
  [left_line]
    type = GenericConstantMaterial
    block = 'left_line'
    prop_names =  'thermal_conductivity specific_heat density'
    prop_values = '0.1                 1.0           1.0' # W/(cm K), J/(g K), g/cm^3
  []
  [right_line]
    type = GenericConstantMaterial
    block = 'right_line'
    prop_names =  'thermal_conductivity specific_heat density'
    prop_values = '5.0e-3                  1.0           1.0' # W/(cm K), J/(g K), g/cm^3
  []
[]

[BCs]
  [right]
    type = FunctionDirichletBC
    variable = temperature
    boundary = 'right'
    function = '10*t'
  []
[]

[VectorPostprocessors]
  [center]
    type = LineValueSampler
    start_point = '-0.5 0 0'
    end_point = '0.5 0 0'
    num_points = 40
    variable = 'temperature'
    sort_by = id
  []
[]

[Executioner]
  type = Transient
  start_time = 0
  dt = 1
  end_time = 1
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
  [csv]
    type = CSV
    file_base = 'csv/line'
    time_data = true
  []
[]
