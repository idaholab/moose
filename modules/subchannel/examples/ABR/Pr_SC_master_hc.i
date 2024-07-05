# This Pronghorn model drives a multiapp setup where wrappers (solid heat conduction)
# and interwrapper flow (fluid) and interwall flow (fluid) and main porous flow are solved in this input.

# Units are SI

# physics parameters
inlet_temperature = 653.15
outlet_pressure = 2.0e5 # Pa # gauge pressure

# geometry parameters
###################################################
# Geometric parameters of XX09
###################################################
# units are cm - do not forget to convert to meter
scale_factor = 0.01
pin_pitch = ${fparse 0.8909*scale_factor}
pin_diameter = ${fparse 0.755*scale_factor}
wire_pitch = ${fparse 20.32*scale_factor}
wire_diameter = ${fparse 0.1307*scale_factor}
flat_to_flat = ${fparse 14.922*scale_factor}
inter_wrapper_width = ${fparse 0.432*scale_factor}
###################################################
#inter_wall_width = ${fparse outer_duct_in - inner_duct_out}

# fluid properties
####  Density #####
A12 = 1.00423e3
A13 = -0.21390
A14 = -1.1046e-5
rho = ${fparse A12 + A13 * inlet_temperature + A14 * inlet_temperature * inlet_temperature}
#### Viscosity
A52 = 3.6522e-5
A53 = 0.16626
A54 = -4.56877e1
A55 = 2.8733e4
mu = ${fparse A52 + A53 / inlet_temperature + A54 / inlet_temperature / inlet_temperature +
        A55 / (inlet_temperature * inlet_temperature * inlet_temperature)}
#### Specific heat at constant pressure
A28 = 7.3898e5
A29 = 3.154e5
A30 = 1.1340e3
A31 = -2.2153e-1
A32 = 1.1156e-4
dt = ${fparse 2503.3 - inlet_temperature}
cp = ${fparse A28 / dt / dt + A29 / dt + A30 + A31 * dt + A32 * dt * dt}
#### Heat conduction coefficient
A48 = 1.1045e2
A49 = -6.5112e-2
A50 = 1.5430e-5
A51 = -2.4617e-9
k = ${fparse A48 + A49 * inlet_temperature + A50 * inlet_temperature * inlet_temperature +
        A51 * inlet_temperature * inlet_temperature * inlet_temperature}
#### Molar mass
molar_mass = 22.989769e-3

# wrapper properties
k_wrapper = 15
cp_wrapper = 300
rho_wrapper = 7800

# sodium properties
k_sodium = 15
cp_sodium = 300
rho_sodium = 7800

# hydraulic diameter
D_hydraulic_interwrapper = ${fparse 2 * inter_wrapper_width}

wrapper_blocks = 'wall'
inter_wrapper_blocks = 'inter_wrapper'

[Mesh]
  [fmesh]
    type = FileMeshGenerator
    file = 'abr_7assemblies_wall.e'
  []
[]

[Problem]
  coord_type = XYZ
[]

[Materials]
  [wall_heat_conductor]
    type = HeatConductionMaterial
    thermal_conductivity = ${k_wrapper}
    block = ${wrapper_blocks}
  []
  [sodium_heat_conductor]
    type = HeatConductionMaterial
    thermal_conductivity = ${k_sodium}
    block = ${inter_wrapper_blocks}
  []
[]

[Variables]
  [T_wrapper]
    order = FIRST
    family = LAGRANGE
  []
[]

[Kernels]
  [heat_conduction]
    type = HeatConduction
    variable = T_wrapper
  []
[]

[BCs]
  [T_wrapper_inside_wall]
    type = MatchedValueBC
    variable = T_wrapper
    boundary = 'wall_in_00 wall_in_01 wall_in_02 wall_in_03
               wall_in_04 wall_in_05 wall_in_06'
    v = duct_surface_temperature
  []
  [outside_bc]
   type = NeumannBC
   variable = T_wrapper
   boundary = '10000 10001 10002'
  []
[]

[AuxVariables]
  [duct_surface_temperature]
    initial_condition = ${inlet_temperature}
  []

  [q_prime_duct]
    order = CONSTANT
    family = MONOMIAL
    block = ${wrapper_blocks}
    initial_condition = 0
  []

  [q_prime_duct_transfer]
    order = CONSTANT
    family = MONOMIAL
    block = ${wrapper_blocks}
    initial_condition = 0
  []
[]

[AuxKernels]
  [QPrime]
    type = QPrimeDuctAux
    diffusivity = ${k_wrapper}
    flat_to_flat = ${flat_to_flat}
    variable = q_prime_duct
    diffusion_variable = T_wrapper
    component = normal
    boundary = 'prsb_interface_00 prsb_interface_01 prsb_interface_02 prsb_interface_03
                prsb_interface_04 prsb_interface_05 prsb_interface_06'
    execute_on = 'initial timestep_end'
  []

  [QPrime_normal_switch]
    type = ParsedAux
    variable = q_prime_duct_transfer
    coupled_variables = q_prime_duct
    expression = '-q_prime_duct'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu superlu_dist'
  fixed_point_max_its   = 5
  fixed_point_min_its = 2
  fixed_point_rel_tol   = 1e-4
  fixed_point_abs_tol   = 1e-3

  [Quadrature]
    order = THIRD
    side_order = FOURTH
  []
[]

[Outputs]
  exodus = true
  csv = true
  print_linear_residuals = false
[]

################################################################################
######    A multiapp that couples Pronghorn to subchannel via duct interface
################################################################################

[MultiApps]
  [subchannel]
    type = FullSolveMultiApp
    input_files = 'fuel_assembly.i'
    execute_on = 'FINAL'
    positions = '       0         0  0
                 -0.08071 -0.139794  0
                  0.08071 -0.139794  0
                  0.16142         0  0
                  0.08071  0.139794  0
                 -0.08071  0.139794  0
                 -0.16142         0  0'
    max_procs_per_app = 1
    output_in_position = true
    bounding_box_padding = '0 0 0.1'
  []
[]
#
[Transfers]
  [q_prime_duct] # Recover q_prime from Pronghorn solve
    type = MultiAppNearestNodeTransfer
    to_multi_app = subchannel
    source_variable = q_prime_duct_transfer
    variable = q_prime_duct
  []

   [T_duct] # Recover T_duct from SC solve
     type = MultiAppNearestNodeTransfer
     from_multi_app = subchannel
     source_variable = Tduct
     variable = duct_surface_temperature
   []
[]
