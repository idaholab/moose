# Apply a piecewise-linear sink flux to the right-hand side and watch fluid flow to it
#
# This test has a single phase with two components.  The test initialises with
# the porous material fully filled with component=1.  The left-hand side is fixed
# at porepressure=1 and mass-fraction of the zeroth component being unity.
# The right-hand side has a very strong piecewise-linear flux that keeps the
# porepressure~0 at that side.  Fluid mass is extracted by this flux in proportion
# to the fluid component mass fraction.
#
# Therefore, the zeroth fluid component will flow from left to right (down the
# pressure gradient).
#
# The important DE is
# porosity * dc/dt = (perm / visc) * grad(P) * grad(c)
# which is true for c = mass-fraction, and very large bulk modulus of the fluid.
# For grad(P) constant in time and space (as in this example) this is just the
# advection equation for c, with velocity = perm / visc / porosity.  The parameters
# are chosen to velocity = 1 m/s.
# In the numerical world, and especially with full upwinding, the advection equation
# suffers from diffusion.  In this example, the diffusion is obvious when plotting
# the mass-fraction along the line, but the average velocity of the front is still
# correct at 1 m/s.
[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 100
  xmin = 0
  xmax = 1
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[UserObjects]
  [./dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pp frac'
    number_fluid_phases = 1
    number_fluid_components = 2
  [../]
[]

[Variables]
  [./pp]
  [../]
  [./frac]
  [../]
[]

[ICs]
  [./pp]
    type = FunctionIC
    variable = pp
    function = 1-x
  [../]
[]

[Kernels]
  [./mass0]
    type = PorousFlowMassTimeDerivative
    fluid_component = 0
    variable = frac
  [../]
  [./mass1]
    type = PorousFlowMassTimeDerivative
    fluid_component = 1
    variable = pp
  [../]
  [./flux0]
    type = PorousFlowAdvectiveFlux
    fluid_component = 0
    gravity = '0 0 0'
    variable = frac
  [../]
  [./flux1]
    type = PorousFlowAdvectiveFlux
    fluid_component = 1
    gravity = '0 0 0'
    variable = pp
  [../]
[]

[Materials]
  [./temperature_nodal]
    type = PorousFlowTemperature
    at_nodes = true
  [../]
  [./temperature_qp]
    type = PorousFlowTemperature
  [../]
  [./ppss_nodal]
    type = PorousFlow1PhaseP_VG
    at_nodes = true
    porepressure = pp
    al = 1 # irrelevant in this fully-saturated test
    m = 0.5
  [../]
  [./ppss]
    type = PorousFlow1PhaseP_VG
    porepressure = pp
    al = 1 # irrelevant in this fully-saturated test
    m = 0.5
  [../]
  [./massfrac_nodal]
    type = PorousFlowMassFraction
    at_nodes = true
    mass_fraction_vars = frac
  [../]
  [./massfrac_qp]
    type = PorousFlowMassFraction
    mass_fraction_vars = frac
  [../]
  [./dens0]
    type = PorousFlowDensityConstBulk
    at_nodes = true
    density_P0 = 1 # almost irrelevant, except that the ability of the right BC to keep P fixed at zero is related to density_P0
    bulk_modulus = 1E10 # need large in order for constant-velocity advection
    phase = 0
  [../]
  [./dens_all]
    type = PorousFlowJoiner
    at_nodes = true
    include_old = true
    material_property = PorousFlow_fluid_phase_density_nodal
  [../]
  [./dens0_qp]
    type = PorousFlowDensityConstBulk
    density_P0 = 1 # almost irrelevant, except that the ability of the right BC to keep P fixed at zero is related to density_P0
    bulk_modulus = 1E10 # need large in order for constant-velocity advection
    phase = 0
  [../]
  [./dens_all_qp]
    type = PorousFlowJoiner
    at_nodes = false
    material_property = PorousFlow_fluid_phase_density_qp
  [../]
  [./porosity]
    type = PorousFlowPorosityConst
    at_nodes = true
    porosity = 0.1
  [../]
  [./porosity_qp]
    type = PorousFlowPorosityConst
    porosity = 0.1
  [../]
  [./permeability]
    type = PorousFlowPermeabilityConst
    at_nodes = false
    permeability = '1.1 0 0 0 1.1 0 0 0 1.1'
  [../]
  [./relperm]
    type = PorousFlowRelativePermeabilityCorey
    at_nodes = true
    n = 2 # irrelevant in this fully-saturated situation
    phase = 0
  [../]
  [./relperm_all]
    type = PorousFlowJoiner
    at_nodes = true
    material_property = PorousFlow_relative_permeability_nodal
  [../]
  [./relperm_qp]
    type = PorousFlowRelativePermeabilityCorey
    n = 2 # irrelevant in this fully-saturated situation
    phase = 0
  [../]
  [./relperm_all_qp]
    type = PorousFlowJoiner
    material_property = PorousFlow_relative_permeability_qp
  [../]
  [./visc0]
    type = PorousFlowViscosityConst
    at_nodes = true
    viscosity = 11 #
    phase = 0
  [../]
  [./visc_all]
    type = PorousFlowJoiner
    at_nodes = true
    material_property = PorousFlow_viscosity_nodal
  [../]
  [./visc0_qp]
    type = PorousFlowViscosityConst
    viscosity = 11 #
    phase = 0
  [../]
  [./visc_all_qp]
    type = PorousFlowJoiner
    material_property = PorousFlow_viscosity_qp
  [../]
[]


[BCs]
  [./lhs_fixed_a]
    type = PresetBC
    boundary = 'left'
    variable = frac
    value = 1
  [../]
  [./lhs_fixed_b]
    type = PresetBC
    boundary = 'left'
    variable = pp
    value = 1
  [../]
  [./flux0]
    type = PorousFlowPiecewiseLinearSink
    boundary = 'right'
    pt_vals = '-100 100'
    multipliers = '-1 1'
    variable = frac # the zeroth comonent
    mass_fraction_component = 0
    use_mobility = false
    use_relperm = false
    fluid_phase = 0
    flux_function = 1E4
  [../]
  [./flux1]
    type = PorousFlowPiecewiseLinearSink
    boundary = 'right'
    pt_vals = '-100 100'
    multipliers = '-1 1'
    variable = pp # comonent 1
    mass_fraction_component = 1
    use_mobility = false
    use_relperm = false
    fluid_phase = 0
    flux_function = 1E4
  [../]
[]

[Preconditioning]
  [./andy]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -sub_pc_type -snes_max_it -sub_pc_factor_shift_type -pc_asm_overlap'
    petsc_options_value = 'gmres asm lu 10000 NONZERO 2'
  [../]
[]

[Executioner]
  type = Transient
  solve_type = Newton
  dt = 1E-2
  end_time = 1
  nl_rel_tol = 1E-9
  nl_abs_tol = 1E-9
[]

[Outputs]
  file_base = s09
  [./console]
    type = Console
    execute_on = 'nonlinear linear'
  [../]
  interval = 10
  exodus = true
[]
