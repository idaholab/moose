rod_diameter = 0.00950
heated_length = 1.0
T_in = 359.15
[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  second_order = true
  [bisonMesh]
    type = GeneratedMeshGenerator
    dim = 2
    xmax = ${fparse rod_diameter / 2.0}
    bias_x = 1.0
    nx = 20
    ymax = ${heated_length}
    ny = 25 # number of axial cells
  []
  coord_type = RZ
  rz_coord_axis = Y
[]

[Functions]
  [volumetric_heat_rate]
    type = ParsedFunction
    expression = '(4.0 * 50000.0 / (pi * D* D * L)) * (pi/2)*sin(pi*y/L)'
    symbol_names = 'L D'
    symbol_values = '${heated_length} ${rod_diameter}'
  []
[]

[Variables]
  [temperature]
    order = SECOND
    family = LAGRANGE
  []
[]

[AuxVariables]
  [Pin_surface_temperature]
  []
  [rod_diameter_deformed]
  []
  [q_prime]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[Modules/TensorMechanics/Master]
  add_variables = true
  strain = SMALL
  incremental = true
  generate_output = 'stress_xx stress_yy stress_xy'
  temperature = temperature

  [block0]
    eigenstrain_names = eigenstrain
    block = 0
  []
[]

[AuxKernels]
  [QPrime]
    type = QPrimeAuxPin
    diffusivity = 'thermal_conductivity'
    rod_diameter = ${fparse rod_diameter}
    variable = q_prime
    diffusion_variable = temperature
    component = normal
    boundary = 'right'
    execute_on = 'timestep_end'
  []
  [Deformation]
    type = ParsedAux
    variable = rod_diameter_deformed
    coupled_variables = 'disp_x'
    expression = '2.0 * disp_x + ${rod_diameter}'
    execute_on = 'FINAL'
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

[Materials]
  [elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    block = 0
    bulk_modulus = 0.333333333333e6
    poissons_ratio = 0.0
  []
  [thermal_strain]
    type = ComputeThermalExpansionEigenstrain
    block = 0
    temperature = temperature
    stress_free_temperature = 117.56
    thermal_expansion_coeff = 1e-6
    eigenstrain_name = eigenstrain
  []
  [stress]
    type = ComputeStrainIncrementBasedStress
    block = 0
  []

  [heat_conductor]
    type = HeatConductionMaterial
    thermal_conductivity = 1.0
    block = 0
  []

  [density]
    type = Density
    block = 0
    density = 1.0
  []
[]

[BCs]
  [left]
    type = NeumannBC
    variable = temperature
    boundary = 'left'
  []
  [right]
    type = MatchedValueBC
    variable = temperature
    boundary = 'right'
    v = Pin_surface_temperature
  []
  [no_x]
    type = DirichletBC
    variable = disp_x
    boundary = 'left'
    value = 0.0
  []
  [no_y]
    type = DirichletBC
    variable = disp_y
    boundary = 'bottom'
    value = 0.0
  []
[]

# [DefaultElementQuality]
#   failure_type = warning
# []

[ICs]
  [temperature_ic]
    type = ConstantIC
    variable = temperature
    value = ${T_in}
  []
  [q_prime_ic]
    type = ConstantIC
    variable = q_prime
    value = 666.0
  []
  [RD_IC]
    type = ConstantIC
    variable = rod_diameter_deformed
    value = ${rod_diameter}
  []
[]

[UserObjects]
  [q_prime_uo]
    type = LayeredSideAverage
    boundary = right
    variable = q_prime
    num_layers = 1000
    direction = y
    execute_on = 'TIMESTEP_END'
  []

  [rod_diameter_uo]
    type = LayeredSideAverage
    boundary = right
    variable = rod_diameter_deformed
    num_layers = 1000
    direction = y
    execute_on = 'TIMESTEP_END'
  []
[]

[Postprocessors]
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'
  nl_abs_tol = 1e-7
  l_max_its = 20
  start_time = 0.0
  dt = 0.5
  num_steps = 2
  end_time = 1.0
  petsc_options_iname = '-pc_type -mat_fd_coloring_err -mat_fd_type'
  petsc_options_value = 'lu       1e-6                 ds'

  [Quadrature]
    order = FIFTH
    side_order = SEVENTH
  []
[]

[Outputs]
  exodus = true
[]
