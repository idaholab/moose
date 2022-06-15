T_in = 630 # K
reactor_power = 250e6 #WTh
fuel_assemblies_per_power_unit = ${fparse 4}
fuel_pins_per_assembly = 217
pin_power = ${fparse reactor_power/(fuel_assemblies_per_power_unit*fuel_pins_per_assembly)} # Approx.

scale_factor = 0.01
fuel_pin_diameter= ${fparse 0.8*scale_factor}
length_entry_fuel = ${fparse 60*scale_factor}
length_heated_fuel = ${fparse 80*scale_factor}
length_outlet_fuel = ${fparse 120*scale_factor}
height = ${fparse length_entry_fuel+length_heated_fuel+length_outlet_fuel}

[Mesh]
  second_order = true
  [rmp]
    type = ReactorMeshParams
    dim = 2
    geom = "Square"
    assembly_pitch = 0.012
    axial_mesh_intervals = '1'
  []

  [pin1]
    type = PinMeshGenerator
    reactor_params = rmp
    pin_type = 1
    pitch = 0.012
    num_sectors = 4
    region_ids='1 2 3 4'
    ring_radii = '${fparse fuel_pin_diameter/2}
                  ${fparse fuel_pin_diameter/2 + 1e-5}
                  ${fparse fuel_pin_diameter/2 + 2e-5}'
    mesh_intervals = '5 1 1 1'
    quad_center_elements = true
  []

  [remove]
    type = BlockDeletionGenerator
    block = '2 3 4'
    input = pin1
  []

  [extrude]
    type = FancyExtruderGenerator
    direction = '0 0 1'
    heights = '${height}'
    input = remove
    num_layers = 10
    bottom_boundary = 1
    top_boundary = 2
  []

  [rename_top_bottom]
    type = RenameBoundaryGenerator
    input = extrude
    old_boundary = '1 2'
    new_boundary = 'bottom top'
  []

  [rename_side]
    type = ParsedGenerateSideset
    input = rename_top_bottom
    new_sideset_name = side
    combinatorial_geometry = 'sqrt(pow(x,2) + pow(y,2)) > ${fparse fuel_pin_diameter/2 - 1e-4}
                              & z > 1e-10 & ${fparse height} - z > 1e-10'
  []

  [rename]
    type = RenameBlockGenerator
    input = rename_side
    old_block = '1'
    new_block = 'fuel_pin'
  []

[]

[Functions]
  [volumetric_heat_rate]
    type = ParsedFunction
    value = 'if(z>l1 & z<l2, 10*power*sin(t*pi/10) + power, 0.0)'
    vars = 'l1 l2 power'
    vals = '${length_entry_fuel} ${fparse length_entry_fuel+length_heated_fuel} ${pin_power}'
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
  [q_prime_pin]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [QPrime]
    type = QPrimeAuxPin
    diffusivity = 'thermal_conductivity'
    rod_diameter = ${fparse fuel_pin_diameter}
    variable = q_prime_pin
    diffusion_variable = temperature
    component = normal
    boundary = 'side'
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
  [time_derivative]
    type = HeatConductionTimeDerivative
    variable = temperature
  []
[]

[Materials]
  [heat_conductor]
    type = HeatConductionMaterial
    thermal_conductivity = 45.0
    block = fuel_pin
  []
  [density]
    type = GenericConstantMaterial
    prop_names = 'density'
    prop_values = 8000.0
  []
[]

[BCs]
  [top_bottom]
    type = NeumannBC
    variable = temperature
    boundary = 'top bottom'
  []
  [side]
    type = MatchedValueBC
    variable = temperature
    boundary = 'side'
    v = Pin_surface_temperature
  []
  # [side]
  #   type = DirichletBC
  #   variable = temperature
  #   boundary = 'side'
  #   value = ${T_in}
  # []
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
  [q_prime_pin_ic]
    type = ConstantIC
    variable = q_prime_pin
    value = 666.0
  []
  [Pin_surface_temperature_ic]
    type = ConstantIC
    variable = Pin_surface_temperature
    value = ${T_in}
  []
[]

[UserObjects]
  [q_prime_pin_uo]
    type = LayeredSideAverage
    boundary = side
    variable = q_prime_pin
    num_layers = 1000
    direction = y
    execute_on = 'TIMESTEP_END'
  []
[]

[Problem]
  coord_type = XYZ
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu superlu_dist'
  end_time = 10
  dt = 1
  nl_abs_tol = 1e-8
  nl_max_its = 15

  [Quadrature]
    order = FIFTH
    side_order = SEVENTH
  []
[]

[Outputs]
  exodus = true
[]
