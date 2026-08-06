# Analytic verification of SCMFrictionPressureDrop with:
#   - three axial cells,
#   - axially varying flow area and wetted perimeter,
#   - central-difference interpolation,
#   - laminar MATRA wall friction,
#   - two local form losses.
#
# SCMQuadAssemblyMeshGenerator requires at least two channels. The two channels
# in this problem are identical, so there is no lateral pressure imbalance and
# no crossflow. At each axial level,
#
#   (F_1 + F_2) / (S_1 + S_2) = F_channel / S_mid,
#
# making the assembly result analytically equivalent to either channel while
# still exercising the cross-sectional homogenization loop.

length = 3.0
n_cells = 3
dz = '${fparse length / n_cells}'

# Nodal geometry varies linearly with z:
#   S(z) = S_0 (1 + area_slope z)
#   P_w(z) = P_0 (1 + perimeter_slope z)
flow_area_0 = 1.0e-4
area_slope = 0.10
wetted_perimeter_0 = 4.0e-2
perimeter_slope = 0.05

rho_const = 1000.0
mu_const = 1.0e-3
mass_flux_in = 80.0
mdot_const = '${fparse mass_flux_in * flow_area_0}'

# Positive flow selects k_grid[iz - 1]. These spacer positions therefore add
# K_1 to cell 1 and K_3 to cell 3; cell 2 has no local loss.
K_1 = 0.50
K_2 = 0.00
K_3 = 1.25

# Geometry at the four axial nodes.
S_0 = '${fparse flow_area_0 * (1.0 + area_slope * 0.0)}'
S_1 = '${fparse flow_area_0 * (1.0 + area_slope * 1.0)}'
S_2 = '${fparse flow_area_0 * (1.0 + area_slope * 2.0)}'
S_3 = '${fparse flow_area_0 * (1.0 + area_slope * 3.0)}'

P_0 = '${fparse wetted_perimeter_0 * (1.0 + perimeter_slope * 0.0)}'
P_1 = '${fparse wetted_perimeter_0 * (1.0 + perimeter_slope * 1.0)}'
P_2 = '${fparse wetted_perimeter_0 * (1.0 + perimeter_slope * 2.0)}'
P_3 = '${fparse wetted_perimeter_0 * (1.0 + perimeter_slope * 3.0)}'

# Central-difference cell properties used by getFrictionPressureDrop().
S_mid_1 = '${fparse 0.5 * (S_0 + S_1)}'
S_mid_2 = '${fparse 0.5 * (S_1 + S_2)}'
S_mid_3 = '${fparse 0.5 * (S_2 + S_3)}'

P_mid_1 = '${fparse 0.5 * (P_0 + P_1)}'
P_mid_2 = '${fparse 0.5 * (P_1 + P_2)}'
P_mid_3 = '${fparse 0.5 * (P_2 + P_3)}'

D_h_1 = '${fparse 4.0 * S_mid_1 / P_mid_1}'
D_h_2 = '${fparse 4.0 * S_mid_2 / P_mid_2}'
D_h_3 = '${fparse 4.0 * S_mid_3 / P_mid_3}'

Re_1 = '${fparse mdot_const / S_mid_1 * D_h_1 / mu_const}'
Re_2 = '${fparse mdot_const / S_mid_2 * D_h_2 / mu_const}'
Re_3 = '${fparse mdot_const / S_mid_3 * D_h_3 / mu_const}'

# All three Reynolds numbers are below the MATRA transition, so f_D = 64 / Re.
f_D_1 = '${fparse 64.0 / Re_1}'
f_D_2 = '${fparse 64.0 / Re_2}'
f_D_3 = '${fparse 64.0 / Re_3}'

# The explicit momentum formulation reconstructs the channel force as
#
#   F_z = 0.5 (f_D dz / D_h + K) mdot^2 / (S_mid rho).
#
# The postprocessor then divides the summed force by the summed interpolated
# area. Since the two channels are identical, the cell contribution is
#
#   delta_p_z = F_z / S_mid.
wall_dp_1 = '${fparse 0.5 * (f_D_1 * dz / D_h_1) * mdot_const^2 / (rho_const * S_mid_1^2)}'
wall_dp_2 = '${fparse 0.5 * (f_D_2 * dz / D_h_2) * mdot_const^2 / (rho_const * S_mid_2^2)}'
wall_dp_3 = '${fparse 0.5 * (f_D_3 * dz / D_h_3) * mdot_const^2 / (rho_const * S_mid_3^2)}'

form_dp_1 = '${fparse 0.5 * K_1 * mdot_const^2 / (rho_const * S_mid_1^2)}'
form_dp_2 = '${fparse 0.5 * K_2 * mdot_const^2 / (rho_const * S_mid_2^2)}'
form_dp_3 = '${fparse 0.5 * K_3 * mdot_const^2 / (rho_const * S_mid_3^2)}'

analytic_wall_dp = '${fparse wall_dp_1 + wall_dp_2 + wall_dp_3}'
analytic_form_dp = '${fparse form_dp_1 + form_dp_2 + form_dp_3}'
analytic_total_dp = '${fparse analytic_wall_dp + analytic_form_dp}'

T_in = 300.0
P_out = 1.0e5

[QuadSubChannelMesh]
  [sub_channel]
    type = SCMQuadAssemblyMeshGenerator
    nx = 1
    ny = 2
    n_cells = ${n_cells}
    pitch = 0.02
    pin_diameter = 0.01
    side_gap = 0.005
    heated_length = ${length}
    spacer_z = '0.0 2.0'
    spacer_k = '${K_1} ${K_3}'
  []
[]

[Functions]
  [area_fn]
    type = ParsedFunction
    expression = '${flow_area_0} * (1.0 + ${area_slope} * z)'
  []
  [perimeter_fn]
    type = ParsedFunction
    expression = '${wetted_perimeter_0} * (1.0 + ${perimeter_slope} * z)'
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
  interpolation_scheme = central_difference
  gravity = none

  # Preserve the constant thermophysical state used by the analytic result.
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
    type = FunctionIC
    variable = S
    function = area_fn
  []
  [w_perim_ic]
    type = FunctionIC
    variable = w_perim
    function = perimeter_fn
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
    mass_flux = ${mass_flux_in}
    execute_on = timestep_begin
  []
[]

[Postprocessors]
  [friction_pressure_drop]
    type = SCMFrictionPressureDrop
    execute_on = timestep_end
  []

  # Analytic decomposition for easier diagnosis if the test fails.
  [analytic_wall_pressure_drop]
    type = ConstantPostprocessor
    value = ${analytic_wall_dp}
    execute_on = timestep_end
  []
  [analytic_form_pressure_drop]
    type = ConstantPostprocessor
    value = ${analytic_form_dp}
    execute_on = timestep_end
  []
  [analytic_pressure_drop]
    type = ConstantPostprocessor
    value = ${analytic_total_dp}
    execute_on = timestep_end
  []

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

  # Confirms that the solved axial mass flow remains equal to the prescribed
  # inlet value despite the changing flow area.
  [mdot_out]
    type = SubChannelPointValue
    variable = mdot
    index = 0
    height = ${length}
    execute_on = timestep_end
  []
  [analytic_mdot]
    type = ConstantPostprocessor
    value = ${mdot_const}
    execute_on = timestep_end
  []
  [mdot_absolute_error]
    type = ParsedPostprocessor
    pp_names = 'mdot_out analytic_mdot'
    expression = 'abs(mdot_out - analytic_mdot)'
    execute_on = timestep_end
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
