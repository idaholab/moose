[Mesh]
  [rectangle]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 5
    ny = 50
    xmin = -0.5
    xmax = 0.5
    ymin = -1.25
    ymax = 1.25
  []
  [strip]
    type = SubdomainBoundingBoxGenerator
    input = rectangle
    bottom_left = '-0.5 -0.05 0'
    top_right = '0.5 0.05 0'
    block_id = 2
    block_name = 'strip'
    location = INSIDE
  []
  [top_bottom_layers]
    type = SubdomainBoundingBoxGenerator
    input = strip
    bottom_left = '-0.5 -0.05 0'
    top_right = '0.5 0.05 0'
    block_id = 1
    block_name = 'rectangle'
    location = OUTSIDE
  []
[]

[Variables]
  [temperature]
  []
[]

[Kernels]
  [time_derivative]
    type = HeatConductionTimeDerivative
    variable = temperature
  []
  [heat_conduction]
    type = HeatConduction
    variable = temperature
  []
[]

[Materials]
  [block]
    type = GenericConstantMaterial
    block = 'rectangle'
    prop_names =  'thermal_conductivity specific_heat density'
    prop_values = '1.0                 1.0           1.0' # W/(cm K), J/(g K), g/cm^3
  []
  [strip]
    type = GenericConstantMaterial
    block = 'strip'
    prop_names =  'thermal_conductivity specific_heat density'
    prop_values = '10.0                 1.0           1.0' # W/(cm K), J/(g K), g/cm^3
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
  [x_n0_25]
    type = LineValueSampler
    start_point = '-0.25 0 0'
    end_point = '-0.25 1.25 0'
    num_points = 100
    variable = 'temperature'
    sort_by = id
  []
  [x_0_25]
    type = LineValueSampler
    start_point = '0.25 0 0'
    end_point = '0.25 1.25 0'
    num_points = 100
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
    file_base = 'csv/rectangle_w_strip'
    time_data = true
  []
[]
