# Developed flow split of the 19-pin LBE bare-pin triangular lattice of tri_bare.i. The assembly is
# solved in steady state at one inlet mass flux, which gives a bulk Reynolds number of about 3.3e4,
# the same as tri_wire_flow_split.i. The flow split, the mass flux of a subchannel divided by the
# bulk mass flux, of one interior, edge, and corner subchannel is reported at the outlet.

T_in = 359.15 # K
P_out = 4.923e6 # Pa
# Bulk mass flux that gives a bulk Reynolds number of about 3.3e4 with the bulk hydraulic diameter
# of this assembly, about 8.6 mm, and the inlet viscosity of tri_wire_flow_split.i
mass_flux = 1275 # kg/m^2-s
# The inlet mass flux is uniform, and the flow split develops over about 1.5 m, so it is sampled at
# the outlet of the 3 m assembly, where the flow is developed
z_out = 3.0
# Subchannel indices of an interior, an edge, and a corner subchannel, as in tri_bare.i
center = 0
edge = 24
corner = 25
# Number of interior, edge, and corner subchannels of the 3-ring assembly: 6 (nrings - 1)^2,
# 6 (nrings - 1), and 6
n_center = 24
n_edge = 12
n_corner = 6

[TriSubChannelMesh]
  [subchannel]
    type = SCMTriAssemblyMeshGenerator
    nrings = 3
    n_cells = 30
    flat_to_flat = 0.05319936
    heated_length = 3
    pin_diameter = 0.0082
    pitch = 0.01148
    dwire = 0.0
    hwire = 0.0
    spacer_z = '0.0'
    spacer_k = '0.0'
  []
[]

[FluidProperties]
  [water]
    type = Water97FluidProperties
  []
[]

[SubChannel]
  type = TriSubChannel1PhaseProblem
  fp = water
  n_blocks = 1
  P_out = ${P_out}
  compute_density = true
  compute_viscosity = true
  compute_power = false
  full_output = true
  # Monolithic implicit solver. The explicit solver converges the developed turbulent flow split
  # only to about 1% with the default tolerances.
  implicit = true
  segregated = false
  P_tol = 1e-8
  T_tol = 1e-8
  rtol = 1e-10
  atol = 1e-10
  friction_closure = 'Chen'
  mixing_closure = 'constant_beta'
  pin_HTC_closure = 'Dittus-Boelter'
[]

[SCMClosures]
  [Chen]
    type = SCMFrictionChenTodreas
  []
  [constant_beta]
    type = SCMMixingConstantBeta
    beta = 0.006
    # No turbulent exchange of axial momentum, so the developed flow split is set by the friction
    # closure alone
    CT = 0.0
  []
  [Dittus-Boelter]
    type = SCMHTCDittusBoelter
  []
[]

[ICs]
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
  [DP_ic]
    type = ConstantIC
    variable = DP
    value = 0.0
  []
  [rho_ic]
    type = RhoFromPressureTemperatureIC
    variable = rho
    p = ${P_out}
    T = T
    fp = water
  []
  [h_ic]
    type = SpecificEnthalpyFromPressureTemperatureIC
    variable = h
    p = ${P_out}
    T = T
    fp = water
  []
  [mu_ic]
    type = ViscosityIC
    variable = mu
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
    execute_on = 'timestep_begin'
  []
  [mdot_in_bc]
    type = SCMMassFlowRateAux
    variable = mdot
    boundary = inlet
    area = S
    mass_flux = ${mass_flux}
    execute_on = 'timestep_begin'
  []
[]

[Postprocessors]
  [mdot_center]
    type = SubChannelPointValue
    variable = mdot
    index = ${center}
    height = ${z_out}
    outputs = none
  []
  [mdot_edge]
    type = SubChannelPointValue
    variable = mdot
    index = ${edge}
    height = ${z_out}
    outputs = none
  []
  [mdot_corner]
    type = SubChannelPointValue
    variable = mdot
    index = ${corner}
    height = ${z_out}
    outputs = none
  []
  [S_center]
    type = SubChannelPointValue
    variable = S
    index = ${center}
    height = ${z_out}
    outputs = none
  []
  [S_edge]
    type = SubChannelPointValue
    variable = S
    index = ${edge}
    height = ${z_out}
    outputs = none
  []
  [S_corner]
    type = SubChannelPointValue
    variable = S
    index = ${corner}
    height = ${z_out}
    outputs = none
  []
  [w_perim_center]
    type = SubChannelPointValue
    variable = w_perim
    index = ${center}
    height = ${z_out}
    outputs = none
  []
  [w_perim_edge]
    type = SubChannelPointValue
    variable = w_perim
    index = ${edge}
    height = ${z_out}
    outputs = none
  []
  [w_perim_corner]
    type = SubChannelPointValue
    variable = w_perim
    index = ${corner}
    height = ${z_out}
    outputs = none
  []
  # The solver evaluates the bulk Reynolds number with the inlet viscosity
  [mu_in]
    type = SubChannelPointValue
    variable = mu
    index = ${center}
    height = 0.0
    outputs = none
  []
  # Subchannel hydraulic diameters Dh = 4 S / w_perim, as in the solver
  [Dh_center]
    type = ParsedPostprocessor
    expression = '4 * S_center / w_perim_center'
    pp_names = 'S_center w_perim_center'
  []
  [Dh_edge]
    type = ParsedPostprocessor
    expression = '4 * S_edge / w_perim_edge'
    pp_names = 'S_edge w_perim_edge'
  []
  [Dh_corner]
    type = ParsedPostprocessor
    expression = '4 * S_corner / w_perim_corner'
    pp_names = 'S_corner w_perim_corner'
  []
  # Bulk hydraulic diameter, 4 times the total flow area over the total wetted perimeter
  [Dh_bulk]
    type = ParsedPostprocessor
    expression = '4 * (${n_center} * S_center + ${n_edge} * S_edge + ${n_corner} * S_corner)
                  / (${n_center} * w_perim_center + ${n_edge} * w_perim_edge
                     + ${n_corner} * w_perim_corner)'
    pp_names = 'S_center S_edge S_corner w_perim_center w_perim_edge w_perim_corner'
  []
  [Re_bulk]
    type = ParsedPostprocessor
    expression = '${mass_flux} * Dh_bulk / mu_in'
    pp_names = 'Dh_bulk mu_in'
  []
  # Flow split, the subchannel mass flux divided by the bulk mass flux
  [X_center]
    type = ParsedPostprocessor
    expression = 'mdot_center / (S_center * ${mass_flux})'
    pp_names = 'mdot_center S_center'
  []
  [X_edge]
    type = ParsedPostprocessor
    expression = 'mdot_edge / (S_edge * ${mass_flux})'
    pp_names = 'mdot_edge S_edge'
  []
  [X_corner]
    type = ParsedPostprocessor
    expression = 'mdot_corner / (S_corner * ${mass_flux})'
    pp_names = 'mdot_corner S_corner'
  []
[]

[Outputs]
  csv = true
  execute_on = final
[]

[Executioner]
  type = Steady
[]
