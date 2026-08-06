# Smallest valid SCMQuadAssemblyMeshGenerator case.
# The mesh generator requires at least 2 x 1 or 1 x 2 channels, so this uses
# two identical, uncoupled channels. The homogenized pressure drop is therefore
# exactly the pressure drop of either channel.

length = 2.0                 # m
flow_area = 1.0e-4           # m^2
wetted_perimeter = 4.0e-2    # m
hydraulic_diameter = '${fparse 4.0 * flow_area / wetted_perimeter}'

rho_const = 1000.0           # kg/m^3
mu_const = 1.0e-3            # Pa-s
mass_flux = 100.0             # kg/(m^2-s)

# Re = G D_h / mu = 1000, so SCMFrictionMATRA uses f_D = 64 / Re.
reynolds = '${fparse mass_flux * hydraulic_diameter / mu_const}'
friction_factor = '${fparse 64.0 / reynolds}'

# Put one local form loss at the inlet. For positive flow and one axial cell,
# SCMFrictionPressureDrop selects k_grid[0].
form_loss_k = 1.0

# Darcy-Weisbach plus local form loss:
#   delta_p = 0.5 * (f_D L / D_h + K) * G^2 / rho
# With the values above, delta_p = 69 Pa.
analytic_delta_p = '${fparse 0.5 * (friction_factor * length / hydraulic_diameter + form_loss_k) * mass_flux^2 / rho_const}'

T_in = 300.0
P_out = 1.0e5

[QuadSubChannelMesh]
  [sub_channel]
    type = SCMQuadAssemblyMeshGenerator
    nx = 1
    ny = 2
    n_cells = 1
    pitch = 0.02
    pin_diameter = 0.01
    side_gap = 0.005
    heated_length = ${length}
    spacer_z = '0.0'
    spacer_k = '${form_loss_k}'
  []
[]

[FluidProperties]
  [water]
    type = Water97FluidProperties
  []
[]

[SubChannel]
  type = QuadSubChannel1PhaseProblem
  fp = water
  n_blocks = 1

  implicit = false
  gravity = none

  # Keep rho and mu equal to their constant IC values so the analytic result is exact.
  compute_density = false
  compute_viscosity = false
  compute_power = false

  P_out = ${P_out}
  friction_closure = MATRA
  mixing_closure = no_mixing
[]

[SCMClosures]
  [MATRA]
    type = SCMFrictionMATRA
  []
  [no_mixing]
    type = SCMMixingConstantBeta
    beta = 0.0
    CT = 0.0
  []
[]

[ICs]
  [S_ic]
    type = ConstantIC
    variable = S
    value = ${flow_area}
  []
  [w_perim_ic]
    type = ConstantIC
    variable = w_perim
    value = ${wetted_perimeter}
  []
  [q_prime_ic]
    type = ConstantIC
    variable = q_prime
    value = 0.0
  []
  [T_ic]
    type = ConstantIC
    variable = T
    value = ${T_in}
  []
  [P_ic]
    type = ConstantIC
    variable = P
    value = 0.0
  []
  [mu_ic]
    type = ConstantIC
    variable = mu
    value = ${mu_const}
  []
  [rho_ic]
    type = ConstantIC
    variable = rho
    value = ${rho_const}
  []
  [h_ic]
    type = SpecificEnthalpyFromPressureTemperatureIC
    variable = h
    p = ${P_out}
    T = T
    fp = water
  []
  [mdot_ic]
    type = ConstantIC
    variable = mdot
    value = 0.0
  []
[]

[AuxKernels]
  [T_in_bc]
    type = ConstantAux
    variable = T
    boundary = inlet
    value = ${T_in}
    execute_on = timestep_begin
  []
  [mdot_in_bc]
    type = SCMMassFlowRateAux
    variable = mdot
    boundary = inlet
    area = S
    mass_flux = ${mass_flux}
    execute_on = timestep_begin
  []
[]

[Postprocessors]
  # Value under test.
  [friction_pressure_drop]
    type = SCMFrictionPressureDrop
    execute_on = timestep_end
  []

  # Closed-form expected value: 69 Pa.
  [analytic_pressure_drop]
    type = ConstantPostprocessor
    value = ${analytic_delta_p}
    execute_on = timestep_end
  []

  # Direct numeric comparison with the analytic value.
  [absolute_error]
    type = ParsedPostprocessor
    pp_names = 'friction_pressure_drop analytic_pressure_drop'
    expression = 'abs(friction_pressure_drop - analytic_pressure_drop)'
    execute_on = timestep_end
  []
  [relative_error]
    type = ParsedPostprocessor
    pp_names = 'friction_pressure_drop analytic_pressure_drop'
    expression = 'abs(friction_pressure_drop - analytic_pressure_drop) / analytic_pressure_drop'
    execute_on = timestep_end
  []

  # Independent balance check using the solved relative pressure field.
  [pressure_in]
    type = SubChannelPointValue
    variable = P
    index = 0
    height = 0.0
    execute_on = timestep_end
  []
  [pressure_out]
    type = SubChannelPointValue
    variable = P
    index = 0
    height = ${length}
    execute_on = timestep_end
  []
  [solved_pressure_drop]
    type = ParsedPostprocessor
    pp_names = 'pressure_in pressure_out'
    expression = 'abs(pressure_in - pressure_out)'
    execute_on = timestep_end
  []
  [solution_absolute_error]
    type = ParsedPostprocessor
    pp_names = 'solved_pressure_drop analytic_pressure_drop'
    expression = 'abs(solved_pressure_drop - analytic_pressure_drop)'
    execute_on = timestep_end
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
