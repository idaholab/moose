# Tests that energy conservation is satisfied in 1-phase flow when there are
# multiple heat structures are connected to the same pipe.
#
# This problem has 2 heat structures with different material properties and
# initial conditions connected to the same flow channel, which has solid wall
# boundary conditions at both ends. An ideal gas equation of state is used for
# the fluid:
#   e(T) = cv * T
# From energy conservation, an analytic expression for the steady-state
# temperature results:
#   (rho(p,T)*e(T)*V)_fluid + (rho*cp*T*V)_hs1 + (rho*cp*T*V)_hs2 = constant
# The following are constant:
#   V_i         domain volumes for flow channel and heat structures
#   rho_fluid   fluid density (due to conservation of mass)
#   rho_hsi     heat structure densities
#   cp_hsi      heat structure specific heats
# Furthermore, all volumes are set equal to 1. Therefore the expression for the
# steady-state temperature is the following:
#   T = E0 / C0
# where
#   E0 = (rho(p0,T0)*e(T0))_fluid + (rho*cp*T0)_hs1 + (rho*cp*T0)_hs2
#   C0 = (rho(p0,T0)*cv)_fluid + (rho*cp)_hs1 + (rho*cp)_hs2
#
# An ideal gas is defined by (gamma, R), and the relation between R and cv is as
# follows:
#   cp = gamma * R / (gamma - 1)
#   cv = cp / gamma = R / (gamma - 1)
# For the EOS parameters
#   gamma = 1.0001
#   R = 100 J/kg-K
# the relevant specific heat is
#   cv = 1e6 J/kg-K
#
# For the initial conditions
#   p = 100 kPa
#   T = 300 K
# the density and specific internal energy should be
#   rho = 3.3333333333333 kg/m^3
#   e = 300000000 J/kg
#
# The following heat structure parameters are used:
#   T0_hs1 = 290 K           T0_hs2 = 310 K
#   rho_hs1 = 8000 kg/m^3    rho_hs2 = 6000 kg/m^3
#   cp_hs1 = 500 J/kg-K      cp_hs2 = 600 J/kg-K
#
# E0 = 1e9 + 8000 * 500 * 290 + 6000 * 600 * 310
#    = 3276000000 J
# C0 = 3.3333333333333e6 + 8000 * 500 + 6000 * 600
#    = 10933333.3333333 J/K
# T = E0 / C0
#   = 3276000000 / 10933333.3333333
#   = 299.6341463414643 K
#

T1 = 290
k1 = 50
rho1 = 8000
cp1  = 500

T2 = 310
k2 = 100
rho2 = 6000
cp2 = 600

[GlobalParams]
  gravity_vector = '0 0 0'

  initial_T = 300
  initial_p = 100e3
  initial_vel = 0

  scaling_factor_1phase = '1e-3 1e-3 1e-8'

  closures = simple_closures
[]

[FluidProperties]
  [fp]
    type = IdealGasFluidProperties
    gamma = 1.0001
    molar_mass = 0.083144598
  []
[]

[Closures]
  [simple_closures]
    type = Closures1PhaseSimple
  []
[]

[HeatStructureMaterials]
  [hs1_mat]
    type = SolidMaterialProperties
    k = ${k1}
    rho = ${rho1}
    cp = ${cp1}
  []
  [hs2_mat]
    type = SolidMaterialProperties
    k = ${k2}
    rho = ${rho2}
    cp = ${cp2}
  []
[]

[Components]
  [pipe]
    type = FlowChannel1Phase
    position = '0 0 0'
    orientation = '1 0 0'
    length = 1
    n_elems = 10
    A = 1
    f = 0
    fp = fp
  []

  [hs1]
    type = HeatStructurePlate
    position = '0 -1 0'
    orientation = '1 0 0'
    length = 1
    depth = 1
    n_elems = 10

    materials = 'hs1_mat'
    n_part_elems = '5'
    widths = '1'
    names = 'solid'

    initial_T = ${T1}
  []

  [hs2]
    type = HeatStructurePlate
    position = '0 -1 0'
    orientation = '1 0 0'
    length = 1
    depth = 1
    n_elems = 10

    materials = 'hs2_mat'
    n_part_elems = '5'
    widths = '1'
    names = 'solid'

    initial_T = ${T2}
  []

  [ht1]
    type = HeatTransferFromHeatStructure1Phase
    hs = hs1
    hs_side = outer
    flow_channel = pipe
    Hw = 1e5
    P_hf = 0.5
  []

  [ht2]
    type = HeatTransferFromHeatStructure1Phase
    hs = hs2
    hs_side = outer
    flow_channel = pipe
    Hw = 1e5
    P_hf = 0.5
  []

  [left]
    type = SolidWall1Phase
    input = 'pipe:in'
  []

  [right]
    type = SolidWall1Phase
    input = 'pipe:out'
  []
[]

[Preconditioning]
  [preconditioner]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  scheme = 'bdf2'

  start_time = 0
  end_time = 4e5
  dt = 1e4
  abort_on_solve_fail = true

  solve_type = 'newton'
  line_search = 'basic'
  nl_rel_tol = 0
  nl_abs_tol = 1e-6
  nl_max_its = 10

  l_tol = 1e-3
  l_max_its = 100

  [Quadrature]
    type = GAUSS
    order = SECOND
  []

  petsc_options_iname = '-pc_type'
  petsc_options_value = ' lu'
[]

[Postprocessors]
  [T_steady_state_predicted]
    type = FunctionValuePostprocessor
    # This value is computed in the input file description
    function = 299.6341463414643
  []
  [T_fluid_average]
    type = ElementAverageValue
    variable = T
    block = pipe
  []
  [relative_error]
    type = RelativeDifferencePostprocessor
    value1 = T_steady_state_predicted
    value2 = T_fluid_average
  []
[]

[Outputs]
  [out]
    type = CSV
    show = 'relative_error'
    execute_on = 'final'
  []
[]
