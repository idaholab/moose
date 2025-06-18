pin_diameter = 0.012065
heated_length = 1.0

[Mesh]
  second_order = true
  [myMesh]
    type = GeneratedMeshGenerator
    dim = 2
    xmax = 0.0060325 # pin diameter / 2.0
    bias_x = 1.0
    nx = 20
    ymax = 1.0 # heated length
    ny = 10 # number of axial cells
  []
  coord_type = RZ
  rz_coord_axis = Y
  beta_rotation = 90
[]

[Functions]
  [volumetric_heat_rate] # Defined such as to be consistent with the IC in SCM
      type = ParsedFunction
      expression = '(4.0 * 1000 / (pi * D * D * L)) * (pi/2)*sin(pi*y/L)'
      symbol_names = 'L D'
      symbol_values = '${heated_length} ${pin_diameter}'
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
  [q_prime]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [QPrime]
    type = SCMRZPinQPrimeAux
    diffusivity = 'thermal_conductivity'
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
    function = volumetric_heat_rate
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
    type = MatchedValueBC
    variable = temperature
    boundary = 'right'
    v = Pin_surface_temperature
  []
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
