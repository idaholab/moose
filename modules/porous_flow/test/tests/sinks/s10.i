# apply a basic sink fluxes to all boundaries.
# Sink strength = S kg.m^-2.s^-1
#
# Use fully-saturated physics, with no flow
# (permeability is zero).
# Each finite element is (2m)^3 in size, and
# porosity is 0.125, so each element holds 1 m^3
# of fluid.
# With density = 10 exp(pp)
# then each element holds 10 exp(pp) kg of fluid
#
# Each boundary node that is away from other boundaries
# (ie, not on a mesh corner or edge) therefore holds
# 5 exp(pp)
# kg of fluid, which is just density * porosity * volume_of_node
#
# Each of such nodes are exposed to a sink flux of strength
# S * A
# where A is the area controlled by the node (in this case 4 m^2)
#
# So d(5 exp(pp))/dt = -4S, ie
# exp(pp) = exp(pp0) - 0.8 * S * t
#
# This is therefore similar to s01.i .  However, this test is
# run 6 times: one for each boundary.  The purpose of this is
# to ensure that the PorousFlowSink BC removes fluid from the
# correct nodes.  This is nontrivial because of the upwinding
# and storing of Material Properties at nodes.
[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 5
  ny = 5
  nz = 5
  xmin = 0
  xmax = 10
  ymin = 0
  ymax = 10
  zmin = 0
  zmax = 10
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pp'
    number_fluid_phases = 1
    number_fluid_components = 1
  []
  [pc]
    type = PorousFlowCapillaryPressureVG
    m = 0.5
    alpha = 1
  []
[]

[Variables]
  [pp]
    initial_condition = 1
  []
[]

[Kernels]
  [mass0]
    type = PorousFlowMassTimeDerivative
    fluid_component = 0
    variable = pp
  []
[]

[FluidProperties]
  [simple_fluid]
    type = SimpleFluidProperties
    bulk_modulus = 1
    density0 = 10
    thermal_expansion = 0
    viscosity = 11
  []
[]

[Materials]
  [temperature]
    type = PorousFlowTemperature
  []
  [ppss]
    type = PorousFlow1PhaseP
    porepressure = pp
    capillary_pressure = pc
  []
  [massfrac]
    type = PorousFlowMassFraction
  []
  [simple_fluid]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid
    phase = 0
  []
  [porosity]
    type = PorousFlowPorosityConst
    porosity = 0.125
  []
[]

[BCs]
  [flux]
    type = PorousFlowSink
    boundary = left
    variable = pp
    use_mobility = false
    use_relperm = false
    fluid_phase = 0
    flux_function = 1
  []
[]

[Preconditioning]
  [andy]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -sub_pc_type -snes_max_it -sub_pc_factor_shift_type -pc_asm_overlap'
    petsc_options_value = 'gmres asm lu 10000 NONZERO 2'
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  dt = 0.25
  end_time = 1
  nl_rel_tol = 1E-12
  nl_abs_tol = 1E-12
[]

[Outputs]
  file_base = s10
  [exodus]
    type = Exodus
    execute_on = 'initial final'
  []
[]
