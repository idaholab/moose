[Mesh]
  parallel_type = 'replicated'
  [block]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 3
    ny = 50
    nz = 1
    xmin = -0.5
    xmax = 0.5
    ymin = -1.25
    ymax = 1.25
    zmin = -0.04
    zmax = 0.04
    boundary_name_prefix = block
  []
  [block_id]
    type = SubdomainIDGenerator
    input = block
    subdomain_id = 1
  []
  [line]
    type = GeneratedMeshGenerator
    dim = 1
    xmin = -0.5
    xmax = 0.5
    nx = 10
    boundary_name_prefix = line
    boundary_id_offset = 10
  []
  [line_id]
    type = SubdomainIDGenerator
    input = line
    subdomain_id = 2
  []
  [combined]
    type = MeshCollectionGenerator
    inputs = 'block_id line_id'
  []
  [line_rename]
    type = RenameBlockGenerator
    input = combined
    old_block = '1 2'
    new_block = 'block line'
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
    block = 'block'
  []
  [heat_conduction]
    type = HeatConduction
    variable = temperature
    block = 'block'
  []
  [time_derivative_line]
    type = TrussHeatConductionTimeDerivative
    variable = temperature
    area = area
    block = 'line'
  []
  [heat_conduction_line]
    type = TrussHeatConduction
    variable = temperature
    area = area
    block = 'line'
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
    value = 0.008
    execute_on = 'initial timestep_begin'
  []
[]

[Constraints]
  [equalvalue]
    type = EqualValueEmbeddedConstraint
    secondary = 'line'
    primary = 'block'
    penalty = 1e6
    formulation = kinematic
    primary_variable = temperature
    variable = temperature
  []
[]

[Materials]
  [block]
    type = GenericConstantMaterial
    block = 'block'
    prop_names =  'thermal_conductivity specific_heat density'
    prop_values = '1.0                 1.0           1.0' # W/(cm K), J/(g K), g/cm^3
  []
  [line]
    type = GenericConstantMaterial
    block = 'line'
    prop_names =  'thermal_conductivity specific_heat density'
    prop_values = '10.0                 1.0           1.0' # W/(cm K), J/(g K), g/cm^3
  []
[]

[BCs]
  [right]
    type = FunctionDirichletBC
    variable = temperature
    boundary = 'block_right line_right'
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
    file_base = 'csv/block_w_line'
    time_data = true
  []
[]
