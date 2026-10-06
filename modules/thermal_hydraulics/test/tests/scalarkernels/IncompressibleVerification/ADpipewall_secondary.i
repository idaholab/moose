pi = 3.14159265358979

length = 1.0

Di = 0.05
Do = 0.06
Dm = '${fparse ( ${Di} + ${Do} ) / 2}'

k_wall = 15.0
cp_wall = 500.0
rho_wall = 8000.0

Pin = 101300
m1 = 1.0
Tf1 = 293.15
m2 = 0.5
Tf2 = 283.15

# Representative water properties, used only to build the independent
# analytical steady-state cross-check below (same constants as the other
# IncompressibleVerification heat tests, e.g. ADheat.i). Both streams are
# water in this test, evaluated with the same representative constants.
mu_f = 0.001002
cp_f = 4185.0
k_f = 0.598

flow_area1 = '${fparse ${pi} / 4 * ${Di} ^ 2}'
wetted_perimeter1 = '${fparse ${pi} * ${Di}}'
Dh1 = '${fparse 4 * ${flow_area1} / ${wetted_perimeter1}}'
area_in = '${fparse ${pi} / 4 * ( ${Dm} ^ 2 - ${Di} ^ 2 )}'
area_out = '${fparse ${pi} / 4 * ( ${Do} ^ 2 - ${Dm} ^ 2 )}'
interface_perimeter = '${fparse ${pi} * ${Dm}}'
interface_thickness = '${fparse ( ${Do} - ${Di} ) / 2}'

flow_area2 = 0.003
wetted_perimeter2 = 0.3
Dh2 = '${fparse 4 * ${flow_area2} / ${wetted_perimeter2}}'

# Independent analytical steady state: a series thermal-resistance circuit
# Tf1 --[primary convection]-- Tin_wall --[radial conduction]-- Tout_wall --[secondary convection]-- Tf2
Re1 = '${fparse ${m1} / ${flow_area1} * ${Dh1} / ${mu_f}}'
Pr1 = '${fparse ${mu_f} * ${cp_f} / ${k_f}}'
h1 = '${fparse 0.023 * ${Re1} ^ 0.8 * ${Pr1} ^ 0.4 * ${k_f} / ${Dh1}}'
G1 = '${fparse ${h1} * ${wetted_perimeter1}}'

G2 = '${fparse ${k_wall} * ${interface_perimeter} / ${interface_thickness}}'

Re2 = '${fparse ${m2} / ${flow_area2} * ${Dh2} / ${mu_f}}'
Pr2 = '${fparse ${mu_f} * ${cp_f} / ${k_f}}'
h2 = '${fparse 0.023 * ${Re2} ^ 0.8 * ${Pr2} ^ 0.4 * ${k_f} / ${Dh2}}'
G3 = '${fparse ${h2} * ${wetted_perimeter2}}'

q = '${fparse ( ${Tf1} - ${Tf2} ) / ( 1 / ${G1} + 1 / ${G2} + 1 / ${G3} )}'
Tin_analytical = '${fparse ${Tf1} - ${q} / ${G1}}'
Tout_analytical = '${fparse ${Tf2} + ${q} / ${G3}}'

[Mesh]
  type = GeneratedMesh
  dim = 1
  xmin = 0
  xmax = ${length}
  nx = 1
[]

[Variables]
  [m1v]
    family = SCALAR
    initial_condition = ${m1}
  []
  [Tf1v]
    family = SCALAR
    initial_condition = ${Tf1}
  []
  [m2v]
    family = SCALAR
    initial_condition = ${m2}
  []
  [Tf2v]
    family = SCALAR
    initial_condition = ${Tf2}
  []
  [Tin_wall]
    family = SCALAR
    initial_condition = ${Tf1}
  []
  [Tout_wall]
    family = SCALAR
    initial_condition = ${Tf2}
  []
[]

[FluidProperties]
  [water]
    type = Water97FluidProperties
  []
[]

[SolidProperties]
  [wall_sp]
    type = ThermalFunctionSolidProperties
    k = ${k_wall}
    cp = ${cp_wall}
    rho = ${rho_wall}
  []
[]

[ScalarKernels]
  [m1_const]
    type = ParsedODEKernel
    expression = 'm1v - ${m1}'
    variable = m1v
  []
  [Tf1_const]
    type = ParsedODEKernel
    expression = 'Tf1v - ${Tf1}'
    variable = Tf1v
  []
  [m2_const]
    type = ParsedODEKernel
    expression = 'm2v - ${m2}'
    variable = m2v
  []
  [Tf2_const]
    type = ParsedODEKernel
    expression = 'Tf2v - ${Tf2}'
    variable = Tf2v
  []

  [inner]
    type = PipeInnerWallTemperatureScalarKernel
    variable = Tin_wall
    mass_flow_rate = m1v
    outer_wall_temperature = Tout_wall
    upstream_wall_temperature = Tin_wall
    downstream_wall_temperature = Tin_wall
    fluid_temperature = Tf1v
    upstream_fluid_temperature = Tf1v
    downstream_fluid_temperature = Tf1v
    sp = wall_sp
    reference_pressure = ${Pin}
    fp = water
    flow_area = ${flow_area1}
    wetted_perimeter = ${wetted_perimeter1}
    area = ${area_in}
    interface_perimeter = ${interface_perimeter}
    interface_thickness = ${interface_thickness}
    length = ${length}
    upstream_spacing = ${length}
    downstream_spacing = ${length}
    upstream_area = ${area_in}
    downstream_area = ${area_in}
    is_implicit = true
  []
  [inner_dt]
    type = ODETimeDerivative
    variable = Tin_wall
  []

  [outer]
    type = PipeOuterWallCoupledConvectiveTemperatureScalarKernel
    variable = Tout_wall
    mass_flow_rate = m2v
    inner_wall_temperature = Tin_wall
    upstream_wall_temperature = Tout_wall
    downstream_wall_temperature = Tout_wall
    fluid_temperature = Tf2v
    upstream_fluid_temperature = Tf2v
    downstream_fluid_temperature = Tf2v
    sp = wall_sp
    reference_pressure = ${Pin}
    fp = water
    flow_area = ${flow_area2}
    wetted_perimeter = ${wetted_perimeter2}
    area = ${area_out}
    interface_perimeter = ${interface_perimeter}
    interface_thickness = ${interface_thickness}
    length = ${length}
    upstream_spacing = ${length}
    downstream_spacing = ${length}
    upstream_area = ${area_out}
    downstream_area = ${area_out}
    is_implicit = true
  []
  [outer_dt]
    type = ODETimeDerivative
    variable = Tout_wall
  []
[]

[Postprocessors]
  [Tin]
    type = ScalarVariable
    variable = Tin_wall
    execute_on = 'TIMESTEP_END'
  []
  [Tout]
    type = ScalarVariable
    variable = Tout_wall
    execute_on = 'TIMESTEP_END'
  []
  [relative_error_in]
    type = ParsedPostprocessor
    expression = 'abs((${Tin_analytical} - Tin) / ${Tin_analytical})'
    pp_names = 'Tin'
    execute_on = 'TIMESTEP_END'
  []
  [relative_error_out]
    type = ParsedPostprocessor
    expression = 'abs((${Tout_analytical} - Tout) / ${Tout_analytical})'
    pp_names = 'Tout'
    execute_on = 'TIMESTEP_END'
  []
[]

[Executioner]
  type = Transient
  start_time = 0
  end_time = 100.0
  [TimeStepper]
    type = IterationAdaptiveDT
    growth_factor = 1.4
    dt = 5
  []
  solve_type = 'PJFNK'
  nl_abs_tol = 1e-08
  l_tol = 1e-07
[]

[Outputs]
  perf_graph = true
  [out]
    type = CSV
    execute_on = 'FINAL'
  []
[]
