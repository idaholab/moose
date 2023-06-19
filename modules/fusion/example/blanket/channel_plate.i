T_in = 623 # K  Table II of the paper: ARIES-ACT2 DCLL Power Core Design and Engineering. T_out = 743.15 K
p_out = 8e+6 # Pa Page 199. Item 3 How would the FW and all blanket structures be efficiently cooled by helium with high operating pressures of ~8 to 10 MPa to remove *46% of total thermal.

width = 0.017 # m
height = 0.01 # m,  square channel
wetted_perimeter = ${fparse 2 * (width+height)} # perimeter
area = ${fparse width * height} # area
#vel = 28 # m/s
# TODO: might use FluidProperties module to compute rho from temperature and pressure
#rho = 3.971 # kg/m3, helium, gas, at 600 K and 50 Bar https://www.engineeringtoolbox.com/helium-density-specific-weight-temperature-pressure-d_2090.html
#mfr = ${fparse rho*width * height*vel} # mass flow rate \rho u A
#mfr = 0.0376 # kg/s, Scale from the numbers at Page 1037, thermohydraulics of rib-roughened helium gas running cooling channels for first wall applications
mfr = 0.1128
D_h = ${fparse (4*area)/wetted_perimeter}
#roughness = 2e-4

[Closures]
  [none_closure]
    type = Closures1PhaseNone
  []
[]

[GlobalParams]
  gravity_vector = '0 0 -9.81'
  initial_T = ${T_in}
  initial_p = ${p_out}
  initial_vel = 0
  closures = none_closure
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
    # TODO: to check whether or not we can use  HeliumFluidProperties
    type = IdealGasFluidProperties
    gamma = 1.667   # (cp/cv) cv: 3117.0, cp: 5195.0 (J/kg/K)
    molar_mass = 4.002602e-3 # kg / mole https://byjus.com/questions/what-is-the-molar-mass-of-helium-gas/
    k = 0.251  # W/mK at 427 C https://www.engineersedge.com/heat_transfer/thermal-conductivity-gases.htm
    mu = 3.475e-5  # kg /(m s) at 427 C https://www.engineersedge.com/heat_transfer/thermal-conductivity-gases.htm
  []
#  [h2]
#    type = HeliumFluidProperties
#  []
[]

[Materials]
  [fD_material]
    type = ADWallFrictionChurchillMaterial
    rho = rho
    vel = vel
    D_h = D_h
    mu = mu
    f_D = 'f_D'
 #   roughness = ${roughness} # 200 um. Page 281, Multiphysics modeling of the FW/blanket of the US fusion nuclear science facility
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
#    Nu_scale = 1.6
  []
[]

[AuxVariables]
  [Hw]
    family = MONOMIAL
    order = CONSTANT
  []
  [qdot]
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
#  [qdot]
#    type = ParsedAux
#    variable = qdot
#    function = '-${wetted_perimeter} * Hw * (T_wall - T)'
#    args = 'T Hw T_wall'
#  []
[]

[Components]
  # components for the moderator channel
  [channel]
    type = FilePipe1Phase
    csv_file = 'channel_meshes/OB_rBZ1_plate1_channel1.csv'
    A = ${area}
    D_h = ${D_h}
    fp = h2
 #   roughness = ${roughness}
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

#  [ht]
#    type = HeatTransferFromSpecifiedTemperature1Phase
#    T_wall = 1000
#    flow_channel = channel
#    P_hf = ${wetted_perimeter}
#  []

  [ht]
    type = HeatTransferFromExternalAppTemperature1Phase
    flow_channel = 'channel'
    P_hf = ${wetted_perimeter}
    initial_T_wall = ${T_in}
    #var_type = elemental
  []
[]

[ControlLogic]
  [mfr]
    type = TimeFunctionComponentControl
    component = inlet
    parameter = m_dot
    function = mfr
  []

  #[temperature]
  #  type = TimeFunctionComponentControl
  #  component = ht
  #  parameter = T_wall
  #  function = temperature
  #[]
[]

[Postprocessors]
  [Qdot]
    type = ElementIntegralVariablePostprocessor
    variable = qdot
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
  #end_time = 0.1
  steady_state_detection = true
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
