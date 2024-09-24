# This test has 2 pipes, each surrounded by a cylindrical HS:
#
#   - pipe1: no fin heat transfer enhancement
#   - pipe2: fin heat transfer enhancement

diam = 0.01
area = ${fparse 0.25 * pi * diam^2}

length = 1.0
n_elems = 10

t_hs = 0.02
n_elems_radial = 5

rho_inlet = 1359.792245 # @ T = 300 K, p = 1e5 Pa
vel_inlet = 1.0
T_inlet = 300
p_outlet = 1e5
T_initial_hs = 800
mfr_inlet = ${fparse rho_inlet * vel_inlet * area}

htc = 100

# Suppose that there are 20 rectangular, 1-mm-thick fins of height 1 mm over the length
# of the cooled section.
n_fin = 20
h_fin = 0.001
t_fin = 0.001
A_fin_single = ${fparse (2 * h_fin + t_fin ) * length}
A_fin = ${fparse n_fin * A_fin_single}
A_cooled = ${fparse pi * diam * length}
A_total = ${fparse A_fin + A_cooled - n_fin * t_fin * length}
fin_area_fraction = ${fparse A_fin / A_total}
area_increase_factor = ${fparse A_total / A_cooled}
fin_perimeter_area_ratio = ${fparse (2 * length + 2 * t_fin) / (length * t_fin)}

k_fin = 15.0

[FluidProperties]
  [fp]
    type = StiffenedGasFluidProperties
    gamma = 2.35
    q = -1167e3
    q_prime = 0
    p_inf = 1e9
    cv = 1816
  []
[]

[Closures]
  [simple_closures]
    type = Closures1PhaseSimple
  []
[]

[SolidProperties]
  [sp_ss316]
    type = ThermalSS316Properties
  []
[]

[FunctorMaterials]
  [fin_efficiency_fmat]
    type = FinEfficiencyFunctorMaterial
    fin_height = ${h_fin}
    fin_perimeter_area_ratio = ${fparse fin_perimeter_area_ratio}
    heat_transfer_coefficient = ${htc}
    thermal_conductivity = ${k_fin}
    fin_efficiency_name = fin_efficiency
  []
  [fin_enhancement_fmat]
    type = FinEnhancementFactorFunctorMaterial
    fin_efficiency = fin_efficiency
    fin_area_fraction = ${fin_area_fraction}
    area_increase_factor = ${area_increase_factor}
    fin_enhancement_factor_name = fin_enhancement
  []
[]

[Components]
  # pipe1

  [pipe1_inlet]
    type = InletMassFlowRateTemperature1Phase
    m_dot = ${mfr_inlet}
    T = ${T_inlet}
    input = 'pipe1:in'
  []
  [pipe1]
    type = FlowChannel1Phase
    gravity_vector = '0 0 0'
    position = '0 0 0'
    orientation = '0 0 1'
    length = ${length}
    n_elems = ${n_elems}
    A = ${area}
    initial_T = ${T_inlet}
    initial_p = ${p_outlet}
    initial_vel = ${vel_inlet}
    fp = fp
    closures = simple_closures
    f = 0
    scaling_factor_1phase = '1 1 1e-5'
  []
  [pipe1_outlet]
    type = Outlet1Phase
    p = ${p_outlet}
    input = 'pipe1:out'
  []
  [ht1]
    type = HeatTransferFromHeatStructure1Phase
    flow_channel = pipe1
    hs = hs1
    hs_side = inner
    Hw = ${htc}
  []
  [hs1]
    type = HeatStructureCylindrical
    position = '0 0 0'
    orientation = '0 0 1'
    length = ${length}
    n_elems = ${n_elems}
    inner_radius = ${fparse 0.5 * diam}
    names = 'main'
    solid_properties = 'sp_ss316'
    solid_properties_T_ref = '300'
    widths = '${t_hs}'
    n_part_elems = '${n_elems_radial}'
    initial_T = ${T_initial_hs}
    scaling_factor_temperature = 1e-5
  []

  # pipe 2

  [pipe2_inlet]
    type = InletMassFlowRateTemperature1Phase
    m_dot = ${mfr_inlet}
    T = ${T_inlet}
    input = 'pipe2:in'
  []
  [pipe2]
    type = FlowChannel1Phase
    gravity_vector = '0 0 0'
    position = '0 0.5 0'
    orientation = '0 0 1'
    length = ${length}
    n_elems = ${n_elems}
    A = ${area}
    initial_T = ${T_inlet}
    initial_p = ${p_outlet}
    initial_vel = ${vel_inlet}
    fp = fp
    closures = simple_closures
    f = 0
    scaling_factor_1phase = '1 1 1e-5'
  []
  [pipe2_outlet]
    type = Outlet1Phase
    p = ${p_outlet}
    input = 'pipe2:out'
  []
  [ht2]
    type = HeatTransferFromHeatStructure1Phase
    flow_channel = pipe2
    hs = hs2
    hs_side = inner
    Hw = ${htc}
    scale = fin_enhancement
  []
  [hs2]
    type = HeatStructureCylindrical
    position = '0 0.5 0'
    orientation = '0 0 1'
    length = ${length}
    n_elems = ${n_elems}
    inner_radius = ${fparse 0.5 * diam}
    names = 'main'
    solid_properties = 'sp_ss316'
    solid_properties_T_ref = '300'
    widths = '${t_hs}'
    n_part_elems = '${n_elems_radial}'
    initial_T = ${T_initial_hs}
    scaling_factor_temperature = 1e-5
  []
[]

[Postprocessors]
  [pipe1_T_avg]
    type = ElementAverageValue
    variable = T
    block = 'pipe1'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [pipe2_T_avg]
    type = ElementAverageValue
    variable = T
    block = 'pipe2'
    execute_on = 'INITIAL TIMESTEP_END'
  []

  [hs1_T_avg]
    type = SideAverageValue
    variable = T_solid
    boundary = 'hs1:inner'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [hs2_T_avg]
    type = SideAverageValue
    variable = T_solid
    boundary = 'hs2:inner'
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Preconditioning]
  [pc]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  scheme = 'bdf2'

  end_time = 10.0
  dt = 1.0

  solve_type = NEWTON
  nl_rel_tol = 0
  nl_abs_tol = 1e-6
  nl_max_its = 15

  l_tol = 1e-3
  l_max_its = 10
[]

[Outputs]
  csv = true
[]
