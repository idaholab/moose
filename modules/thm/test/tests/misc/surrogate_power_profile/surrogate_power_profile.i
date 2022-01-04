# This takes an exodus file with a power profile and uses that in a heat structure
# of a core channel as power density.  This tests the capability of taking a
# rattlesnake generated power profile and using it in RELAP-7.

[GlobalParams]
  initial_p = 15.5e6
  initial_vel = 0.
  initial_T = 559.15

  gravity_vector = '0 -9.8 0'

  scaling_factor_1phase = '1 1 1e-4'
  scaling_factor_temperature = 1e-2

  closures = simple_closures
[]

[FluidProperties]
  [water]
    type = StiffenedGasFluidProperties
    gamma = 2.35
    q = -1167e3
    q_prime = 0
    p_inf = 1.e9
    cv = 1816
  []
[]

[Closures]
  [simple_closures]
    type = Closures1PhaseSimple
  []
[]

[HeatStructureMaterials]
  [fuel-mat]
    type = SolidMaterialProperties
    k = 2.5
    cp = 300.
    rho = 1.032e4
  []
  [gap-mat]
    type = SolidMaterialProperties
    k = 0.6
    cp = 1.
    rho = 1.
  []
  [clad-mat]
    type = SolidMaterialProperties
    k = 21.5
    cp = 350.
    rho = 6.55e3
  []
[]

[Components]
  [CCH1:pipe]
    type = FlowChannel1Phase
    position = '0.02 0 0'
    orientation = '0 1 0'
    length = 3.865
    n_elems = 20

    A = 8.78882e-5
    D_h = 0.01179
    f = 0.01
    fp = water
  []

  [CCH1:solid]
    type = HeatStructureCylindrical
    position = '0.024748 0 0'
    orientation = '0 1 0'
    length = 3.865
    n_elems = 20

    initial_T = 559.15
    names = 'fuel gap clad'
    widths = '0.004096 0.0001 0.000552'
    n_part_elems = '5 1 2'
    materials = 'fuel-mat gap-mat clad-mat'
  []

  [CCH1:hx]
    type = HeatTransferFromHeatStructure1Phase
    flow_channel = CCH1:pipe
    hs = CCH1:solid
    hs_side = outer
    Hw = 5.33e4
    P_hf = 2.9832563838489e-2
  []

  [inlet]
    type = InletMassFlowRateTemperature1Phase
    input = 'CCH1:pipe:in'
    m_dot = 0.1
    T = 559.15
  []
  [outlet]
    type = Outlet1Phase
    input = 'CCH1:pipe:out'
    p = 15.5e6
  []
[]

[UserObjects]
  [reactor_power_density_uo]
    type = SolutionUserObject
    mesh = 'power_profile.e'
    system_variables = power_density
    translation = '0. 0. 0.'
  []
[]

[Functions]
  [power_density_fn]
    type = SolutionFunction
    from_variable = power_density
    solution = reactor_power_density_uo
  []
[]

[AuxVariables]
  [power_density]
    family = MONOMIAL
    order = CONSTANT
    block = 'CCH1:solid:fuel'
  []
[]

[AuxKernels]
  [power_density_aux]
    type = FunctionAux
    variable = power_density
    function = power_density_fn
    block = 'CCH1:solid:fuel'
    execute_on = 'timestep_begin'
  []
[]

[Preconditioning]
  [SMP_PJFNK]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  scheme = 'bdf2'

  start_time = 0.0
  num_steps = 10
  dt = 1e-2
  abort_on_solve_fail = true

  solve_type = 'PJFNK'
  line_search = 'basic'
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-9
  nl_max_its = 10

  l_tol = 1e-3
  l_max_its = 100

  [Quadrature]
    type = GAUSS
    order = SECOND
  []
[]

[Outputs]
  [out]
    type = Exodus
  []
  velocity_as_vector = false
[]
