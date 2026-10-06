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
Tamb = 273.15
htc_amb = 50.0
emissivity = 0.8

flow_area1 = '${fparse ${pi} / 4 * ${Di} ^ 2}'
wetted_perimeter1 = '${fparse ${pi} * ${Di}}'
area_in = '${fparse ${pi} / 4 * ( ${Dm} ^ 2 - ${Di} ^ 2 )}'
area_out = '${fparse ${pi} / 4 * ( ${Do} ^ 2 - ${Dm} ^ 2 )}'
interface_perimeter = '${fparse ${pi} * ${Dm}}'
interface_thickness = '${fparse ( ${Do} - ${Di} ) / 2}'
ambient_perimeter = '${fparse ${pi} * ${Do}}'

# Independent analytical steady state: a series thermal-resistance circuit
# Tf1 --[fluid convection]-- Tin_wall --[radial conduction]-- Tout_wall --[parallel ambient
# convection + radiation]-- Tamb
#
# Unlike pipewall_ambient.i, the outer node now loses heat through two parallel paths to
# ambient (convection, linear in T, and radiation, quartic in T), so the steady state is
# the root of a single nonlinear equation in Tout rather than a plain series-resistor
# network. Eliminating Tin via the (linear) inner-node balance collapses the fluid-side and
# radial conductances into a single series conductance Gseries = G1*G2/(G1+G2), leaving:
#   Gseries*(Tf1 - Tout) = htc_amb*ambient_perimeter*(Tout - Tamb)
#                          + sigma*emissivity*ambient_perimeter*(Tout^4 - Tamb^4)
# which was solved for Tout by Newton's method offline (not at parse time, since fparse
# cannot solve a nonlinear equation), then Tin recovered from the linear relation. The
# values below were verified to satisfy both nodes' original flux-balance equations to
# within 1e-11 W/m.
Tin_analytical = 292.53408744418897
Tout_analytical = 292.1600915387281

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
  [Tin_wall]
    family = SCALAR
    initial_condition = ${Tf1}
  []
  [Tout_wall]
    family = SCALAR
    initial_condition = ${Tamb}
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
    type = PipeOuterWallAmbientTemperatureScalarKernel
    variable = Tout_wall
    inner_wall_temperature = Tin_wall
    upstream_wall_temperature = Tout_wall
    downstream_wall_temperature = Tout_wall
    sp = wall_sp
    T_ambient = ${Tamb}
    htc_ambient = ${htc_amb}
    ambient_perimeter = ${ambient_perimeter}
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
  [outer_radiation]
    type = PipeOuterWallRadiationScalarKernel
    variable = Tout_wall
    sp = wall_sp
    area = ${area_out}
    perimeter = ${ambient_perimeter}
    T_ambient = ${Tamb}
    emissivity = ${emissivity}
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
