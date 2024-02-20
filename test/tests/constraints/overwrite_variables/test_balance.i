# Test to exemplify the use of overwriting of variables in the framework.
[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  diffusivity = 1e3
  use_displaced_mesh = true
[]

[Mesh]
  [block_one]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 3
    ny = 3
    nz = 3
    xmin = 4.5
    xmax = 5.5
    ymin = 4.5
    ymax = 5.5
    zmin = 0.0001
    zmax = 1.0001
    boundary_name_prefix = 'ball'
  []
  [block_two]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 2
    ny = 2
    nz = 2
    xmin = 0.0
    xmax = 10
    ymin = 0.0
    ymax = 10
    zmin = -2
    zmax = 0
    boundary_name_prefix = 'base'
    boundary_id_offset = 10
  []
  [block_one_id]
    type = SubdomainIDGenerator
    input = block_one
    subdomain_id = 1
  []
  [block_two_id]
    type = SubdomainIDGenerator
    input = block_two
    subdomain_id = 2
  []
  [combine]
    type = MeshCollectionGenerator
    inputs = ' block_one_id block_two_id'
  []
  allow_renumbering = false
[]

[Problem]
  kernel_coverage_check = false
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
  [disp_z]
  []
[]

[AuxVariables]
  [vel_x]
  []
  [accel_x]
  []
  [vel_y]
  []
  [accel_y]
  []
  [vel_z]
  []
  [accel_z]
  []
  [gap_rate]
  []
[]

[Kernels]
  [disp_x]
    type = MatDiffusion
    variable = disp_x
  []
  [disp_y]
    type = MatDiffusion
    variable = disp_y
  []
  [disp_z]
    type = MatDiffusion
    variable = disp_z
  []
  [vel_x]
    type = TimeDerivative
    variable = disp_x
  []
  [vel_y]
    type = TimeDerivative
    variable = disp_y
  []
  [vel_z]
    type = TimeDerivative
    variable = disp_z
  []
  [source_m]
    type = BodyForce
    variable = disp_z
    value = -100
  []
[]

[BCs]
  [x_front]
    type = DirichletBC
    variable = disp_x
    boundary = 'ball_front'
    preset = false
    value = 0.0
  []
  [y_front]
    type = DirichletBC
    variable = disp_y
    boundary = 'ball_front'
    preset = false
    value = 0.0
  []
  [x_fixed]
    type = DirichletBC
    variable = disp_x
    boundary = 'base_back'
    value = 0.0
    preset = true
  []
  [y_fixed]
    type = DirichletBC
    variable = disp_y
    boundary = 'base_back'
    value = 0.0
    preset = true
  []
  [z_fixed]
    type = DirichletBC
    variable = disp_z
    boundary = 'base_back'
    value = 0.0
    preset = true
  []
  [z_fixed_front]
    type = DirichletBC
    variable = disp_z
    boundary = 'base_front'
    value = 0.0
    preset = true
  []
[]

[Constraints]
  [overwrite]
    type = ExplicitDynamicsOverwrite
    model = frictionless_balance
    primary = base_front
    secondary = ball_back
    vel_x = 'vel_x'
    vel_y = 'vel_y'
    vel_z = 'vel_z'
    primary_variable = disp_x
    boundary = 'base_front'
    component = 0
    variable = disp_x
    gap_rate = gap_rate
  []
[]

[Materials]
  [density_one]
    type = GenericConstantMaterial
    prop_names = density
    prop_values = 1e5
    outputs = 'exodus'
    output_properties = 'density'
    block = '1'
  []
  [density_two]
    type = GenericConstantMaterial
    prop_names = density
    prop_values = 1e5
    outputs = 'exodus'
    output_properties = 'density'
    block = '2'
  []
[]

[Executioner]
  type = Transient
  start_time = -0.01
  end_time = -0.008
  dt = 1.0e-5
  timestep_tolerance = 1e-6

  [TimeIntegrator]
    type = CentralDifference
    solve_type = lumped
  []
[]

[Outputs]
  interval = 50
  exodus = true
  csv = true
[]

[Postprocessors]
[]
