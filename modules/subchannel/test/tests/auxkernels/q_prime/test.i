rod_diameter = 0.4
heated_length = 1.0

[Mesh]
  second_order = true
  [bisonMesh]
    type = GeneratedMeshGenerator
    dim = 2
    xmax = 0.2 # rod diameter / 2.0
    bias_x = 1.0
    nx = 5
    ymax = 1.0 # heated length
    ny = 10 # number of axial cells
  []
[]

[Variables]
  [temperature]
    order = SECOND
    family = LAGRANGE
  []
[]

[AuxVariables]
  [q_prime]
    order = CONSTANT
    family = MONOMIAL
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
[]

[Kernels]
  [heat_conduction]
    type = HeatConduction
    variable = temperature
  []
  [heat_source]
    type = HeatSource
    variable = temperature
    value = ${fparse 4.0 * 1000 / (pi * rod_diameter * rod_diameter * heated_length)}
  []
[]

[Materials]
  [heat_conductor]
    type = HeatConductionMaterial
    thermal_conductivity = 1.0
    block = 0
  []
[]

[BCs]
  [left]
    type = NeumannBC
    variable = temperature
    boundary = 'left'
  []
  [right]
    type = DirichletBC
    variable = temperature
    boundary = 'right'
    value = 200.0
  []
[]

# [DefaultElementQuality]
#   failure_type = warning
# []

[UserObjects]
  [q_prime_uo]
    type = LayeredSideAverage
    boundary = right
    variable = q_prime
    num_layers = 10
    direction = y
    execute_on = 'TIMESTEP_END'
  []
[]

[Problem]
  coord_type = RZ
  rz_coord_axis = Y
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu superlu_dist'
[]

[Outputs]
  exodus = true
[]
