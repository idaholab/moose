# This was one of the test already put in place by Fande. The test uses the custom 1D csv file channel and compares it with a gold file.0

T_in = 658.15 # K
p_out = 8e+6 # Pa

width = 0.02 # m
height = 0.02 # m,  square channel
wetted_perimeter = ${fparse 2 * (width+height)} # perimeter
area = ${fparse width * height} # area
vel = 0.5 # m/s
rho = 0.164 # kg/m3, helium, gas, ~300 K
mfr = ${fparse rho*vel*width * height} # mass flow rate \rho u A. A = 0.004,
D_h = ${fparse (4*area)/wetted_perimeter}

[Closures]
  [my_closures]
    type = Closures1PhaseNone
  []
[]

[GlobalParams]
  gravity_vector = '0 0 -9.81'
  initial_T = ${T_in}
  initial_p = ${p_out}
  initial_vel = 0
  closures = my_closures
[]

[Functions]
  [mfr]
    type = PiecewiseLinear
    x = '0 0.1'
    y = '0 ${mfr}'
  []
[]

[Modules/FluidProperties]
  [h2]
    type = IdealGasFluidProperties
    gamma = 1.667   # (cp/cv) cv: 3117.0, cp: 5195.0 (J/kg/K)
    molar_mass = 4.002602e-3 #
    k = 0.227  # at 316 C https://www.engineersedge.com/heat_transfer/thermal-conductivity-gases.htm
    mu = 3e-5  # at 316 C
  []
[]

[Materials]
  [fD_material]
    type = ADWallFrictionChurchillMaterial
    rho = rho
    vel = vel
    D_h = D_h
    mu = mu
    f_D = 'f_D'
  []

  [Hw_material]
    type = ADWallHeatTransferCoefficient3EqnDittusBoelterMaterial
    rho = rho
    vel = vel
    D_h = D_h
    k = k
    mu = mu
    cp = cp
    T = T
    T_wall = T_wall
  []
[]

[AuxVariables]
  [Hw]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [Hw_aux]
    type = ADMaterialRealAux
    variable = Hw
    property = Hw
  []
[]

[Components]
  # components for the moderator channel
  [channel]
    type = FilePipe1Phase
    csv_file = 'quarter.csv'
    A = ${area}
    D_h = ${D_h}
    fp = h2
  []

  [inlet]
    type = InletMassFlowRateTemperature1Phase
    input = 'channel:in'
    m_dot = 0 # m_dot is controlled by logic function
    T = ${T_in}
  []

  [outlet]
    type = Outlet1Phase
    input = 'channel:out'
    p = ${p_out}
  []

  [ht]
    type = HeatTransferFromSpecifiedTemperature1Phase
    T_wall = 1173.15
    flow_channel = channel
    P_hf = ${wetted_perimeter}
  []
[]

[ControlLogic]
  [mfr]
    type = TimeFunctionComponentControl
    component = inlet
    parameter = m_dot
    function = mfr
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
  start_time = 0
  end_time = 0.1
  solve_type = Newton
  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 0.01
    iteration_window = 3
    optimal_iterations = 6
    growth_factor = 1.05
    cutback_factor = 0.8
  []

  petsc_options_iname = '-pc_type'
  petsc_options_value = ' lu     '
  line_search = l2

  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-4
  nl_max_its = 30
  l_tol = 1e-2
  l_max_its = 30
  automatic_scaling = true
[]

[Outputs]
  print_linear_residuals = false
  print_linear_converged_reason = false
  print_nonlinear_converged_reason = false
  csv = true
  exodus = true
  execute_on = 'timestep_end'
  [console]
    type = Console
    max_rows = 1
  []
[]
