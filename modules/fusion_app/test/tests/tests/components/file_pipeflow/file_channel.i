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

[GlobalParams]
  gravity_vector = '0 0 -9.81'
  initial_T = ${T_in}
  initial_p = ${p_out}
  initial_vel = 0
  closures = none
[]

[Functions]
  [mfr]
    type = PiecewiseLinear
    x = '0 0.1'
    y = '0 ${mfr}'
  []
[]

[FluidProperties]
  [h2]
    type = IdealGasFluidProperties
    gamma = 1.667   # (cp/cv) cv: 3117.0, cp: 5195.0 (J/kg/K)
    molar_mass = 4.002602e-3 #
    k = 0.227  # at 316 C https://www.engineersedge.com/heat_transfer/thermal-conductivity-gases.htm
    mu = 3e-5  # at 316 C
  []
[]

[Materials]
  [f_wall_mat]
    type = ConstantMaterial
    property_name = 'f_D'
    derivative_vars = 'rhoA rhouA rhoEA'
    value = 0.164 # kg/m3, helium, gas, ~300 K
  []

  [Re_mat]
    type = ParsedMaterial
    f_name = 'Re'
    function = 'rho * abs(vel) * D_h / mu'
    material_property_names = 'rho vel D_h mu'
  []

  [Pr_mat]
    type = PrandtlNumberMaterial
    cp = cp
    mu = mu
    k = k
  []

  [Nu_mat]
    type = ParsedMaterial
    f_name = "Nu"
    function = '0.023 * pow(Re, 4 / 5) * pow(Pr, 0.3)' # Page 29 of RELAP Manul
    material_property_names = 'Re Pr T'
  []

  [Hw_mat]
    # Computes convective heat transfer coefficient from Nusselt number
    type = ConvectiveHeatTransferCoefficientMaterial
    Nu = Nu
    k = k
    D_h = ${D_h}
  []
[]

[AuxVariables]
  [Hw]
    family = MONOMIAL
    order = CONSTANT
  []

  [Re]
    family = MONOMIAL
    order = CONSTANT
  []

  [Nu]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [Hw_aux]
    type = MaterialRealAux
    variable = Hw
    property = Hw
  []

  [Re_aux]
    type = MaterialRealAux
    variable = Re
    property = Re
  []

  [Nu_aux]
    type = MaterialRealAux
    variable = Nu
    property = Nu
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
