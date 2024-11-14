# a wrapper mesh for coupling to subchannel

# sqrt(3) / 2 is by how much flat to flat is smaller than corer to corner
f = '${fparse sqrt(3) / 2}'

# units are meters
height = 1.0
duct_inside = 0.085
wrapper_thickness = 0.002
duct_outside = '${fparse duct_inside + 2 * wrapper_thickness}'

# number of radial elements in the wrapper
n_radial = 4

# number of azimuthal elements per side
n_az = 4

# number of axial elements
n_ax = 10

# System variables
T_in = 660
[DefaultElementQuality]
  failure_type = warning
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  [bisonMesh]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 6
    num_sectors_per_side = '${n_az} ${n_az} ${n_az} ${n_az} ${n_az} ${n_az}'
    background_intervals = 1
    background_block_ids = '1'
    # note that polygon_size is "like radius"
    polygon_size = '${fparse duct_outside / 2}'
    duct_sizes = '${fparse duct_inside / 2 / f}'
    duct_intervals = '${n_radial}'
    duct_block_ids = '2'
    # interface_boundary_names = 'inside'
    external_boundary_name = 'outside'
  []

  [extrude]
    type = AdvancedExtruderGenerator
    # type = FancyExtruderGenerator
    direction = '0 0 1'
    input = bisonMesh
    heights = '${height}'
    num_layers = '${n_ax}'
  []

  [inlet_boundary]
    type = ParsedGenerateSideset
    input = extrude
    combinatorial_geometry = 'z < 1e-6'
    normal = '0 0 -1'
    new_sideset_name = 'inlet'
  []

  [outlet_boundary]
    type = ParsedGenerateSideset
    input = inlet_boundary
    combinatorial_geometry = 'z > ${fparse height - 1e-6}'
    normal = '0 0 1'
    new_sideset_name = 'outlet'
  []

  [inside_boundary]
    type = SideSetsBetweenSubdomainsGenerator
    primary_block = 2
    paired_block = 1
    new_boundary = 'inside'
    input = outlet_boundary
  []

  [remove]
    type = BlockDeletionGenerator
    block = 1
    input = inside_boundary
  []

  [rename]
    type = RenameBlockGenerator
    input = remove
    old_block = '2'
    new_block = 'wrapper'
  []

  [rotate]
    type = TransformGenerator
    input = rename
    transform = ROTATE
    vector_value = '30 0 0'
  []

  coord_type = XYZ
[]

[Functions]
  [volumetric_heat_rate]
    type = ParsedFunction
    expression = '1.0*H'
    symbol_names = 'H'
    symbol_values = '${height}'
  []
[]

[Variables]
  [temperature]
    order = FIRST
    family = LAGRANGE
  []
[]

[Modules]

  [TensorMechanics]

    [Master]
      add_variables = true
      strain = SMALL
      incremental = true
      generate_output = 'stress_xx stress_yy stress_xy'
      temperature = temperature

      [block0]
        eigenstrain_names = eigenstrain
        block = wrapper
      []
    []
  []
[]

[Kernels]
  [heat_conduction]
    type = HeatConduction
    variable = temperature
  []
  [heat_source]
    type = HeatSource
    variable = temperature
    function = volumetric_heat_rate
  []
[]

[AuxVariables]
  [q_prime]
    order = CONSTANT
    family = MONOMIAL
  []
  [duct_surface_temperature]
  []
  [disp_magnitude]
  []
[]

[AuxKernels]
  [QPrime]
    type = SCMTriDuctQPrimeAux
    diffusivity = 'thermal_conductivity'
    flat_to_flat = '${fparse duct_inside}'
    variable = q_prime
    diffusion_variable = temperature
    component = normal
    boundary = 'inside'
    execute_on = 'timestep_end'
  []
  [Deformation]
    type = ParsedAux
    variable = disp_magnitude
    coupled_variables = 'disp_x disp_y disp_z'
    expression = 'sqrt(disp_x^2 + disp_y^2 + disp_z^2)'
    execute_on = 'timestep_end'
  []
[]

[Materials]
  [elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    block = wrapper
    bulk_modulus = 0.333333333333e6
    poissons_ratio = 0.0
  []
  [thermal_strain]
    type = ComputeThermalExpansionEigenstrain
    block = wrapper
    temperature = temperature
    stress_free_temperature = ${T_in}
    thermal_expansion_coeff = 1e-5
    eigenstrain_name = eigenstrain
  []
  [stress]
    type = ComputeStrainIncrementBasedStress
    block = wrapper
  []
  [heat_conductor]
    type = HeatConductionMaterial
    thermal_conductivity = 1.0
    block = wrapper
  []
  [density]
    type = Density
    block = wrapper
    density = 1.0
  []
[]

[BCs]
  [isolated_bc]
    type = NeumannBC
    variable = temperature
    boundary = 'inlet outlet'
  []
  [inside_bc]
    type = MatchedValueBC
    variable = temperature
    boundary = 'inside'
    v = duct_surface_temperature
  []
  [outside_bc]
    type = DirichletBC
    variable = temperature
    boundary = 'outside'
    value = '${fparse T_in+10}'
  []

  [no_x]
    type = DirichletBC
    variable = disp_x
    boundary = 'inlet outlet'
    value = 0.0
  []

  [no_y]
    type = DirichletBC
    variable = disp_y
    boundary = 'inlet outlet'
    value = 0.0
  []

  [no_z]
    type = DirichletBC
    variable = disp_z
    boundary = 'inlet'
    value = 0.0
  []
[]

[ICs]
  [temperature_ic]
    type = ConstantIC
    variable = temperature
    value = ${T_in}
  []
  [q_prime_ic]
    type = ConstantIC
    variable = q_prime
    value = 0.0
  []
[]

[UserObjects]
  [q_prime_uo]
    type = LayeredSideAverage
    boundary = 'inside'
    variable = q_prime
    num_layers = 1000
    direction = z
    execute_on = 'TIMESTEP_END'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu superlu_dist'

  [Quadrature]
    order = THIRD
    side_order = FOURTH
  []
[]

[Outputs]
  exodus = true
[]
