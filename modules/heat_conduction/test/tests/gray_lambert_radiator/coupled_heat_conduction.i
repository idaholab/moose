[Problem]
  kernel_coverage_check = false
[]

[Mesh]
  type = MeshGeneratorMesh

  [./cartesian]
    type = CartesianMeshGenerator
    dim = 2
    dx = '1 1 1'
    ix = '2 2 2'
    dy = '5'
    iy = '10'
    subdomain_id = '1 2 3'
  [../]

  [./break_sides]
    type = BreakBoundaryOnSubdomainGenerator
    boundaries = 'bottom top'
    input = cartesian
  [../]

  [./left_interior]
    type = SideSetsBetweenSubdomainsGenerator
    primary_block = 1
    paired_block = 2
    new_boundary = left_interior
    input = break_sides
  [../]

  [./right_interior]
    type = SideSetsBetweenSubdomainsGenerator
    primary_block = 3
    paired_block = 2
    new_boundary = right_interior
    input = left_interior
  [../]
  [./rename]
    type = RenameBlockGenerator
    input = right_interior
    old_block = '1 2 3'
    new_block = '1 4 3'
  [../]
[]


[Variables]
  [./temperature]
    initial_condition = 300
    block = '1 3'
  [../]
[]

[Kernels]
  [./heat_conduction]
    type = HeatConduction
    variable = temperature
    diffusion_coefficient = 1
    block = '1 3'
  [../]
[]

[UserObjects]
  [./cavity_radiation]
    type = ConstantViewFactorSurfaceRadiation
    boundary = 'left_interior right_interior bottom_to_2 top_to_2'
    temperature = temperature
    emissivity = '0.8 0.8 0.8 0.8'
    adiabatic_boundary = 'bottom_to_2 top_to_2'
    # these view factors are made up to exactly balance energy
    # transfer through the cavity
    view_factors = '0    0.8 0.1 0.1;
                    0.8  0   0.1 0.1;
                    0.45 0.45  0 0.1;
                    0.45 0.45 0.1  0'
    execute_on = 'INITIAL LINEAR TIMESTEP_END'
  [../]
[]

[BCs]
  [./bottom_left]
    type = DirichletBC
    preset = false
    variable = temperature
    boundary = bottom_to_1
    value = 1500
  [../]

  [./top_right]
    type = DirichletBC
    preset = false
    variable = temperature
    boundary = top_to_3
    value = 300
  [../]

  [./radiation]
    type = GrayLambertNeumannBC
    variable = temperature
    reconstruct_emission = false
    surface_radiation_object_name = cavity_radiation
    boundary = 'left_interior right_interior'
  [../]
[]

[Postprocessors]
  [./qdot_left]
    type = GrayLambertSurfaceRadiationPP
    boundary = left_interior
    surface_radiation_object_name = cavity_radiation
    return_type = HEAT_FLUX_DENSITY
  [../]

  [./qdot_right]
    type = GrayLambertSurfaceRadiationPP
    boundary = right_interior
    surface_radiation_object_name = cavity_radiation
    return_type = HEAT_FLUX_DENSITY
  [../]

  [./qdot_top]
    type = GrayLambertSurfaceRadiationPP
    boundary = top_to_2
    surface_radiation_object_name = cavity_radiation
    return_type = HEAT_FLUX_DENSITY
  [../]

  [./qdot_bottom]
    type = GrayLambertSurfaceRadiationPP
    boundary = bottom_to_2
    surface_radiation_object_name = cavity_radiation
    return_type = HEAT_FLUX_DENSITY
  [../]
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
