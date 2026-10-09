# Two radial wall nodes bridge a fixed primary-fluid temperature Tf1 to a prescribed
# outer-wall heat flux q_flux (PipeOuterWallHeatFluxScalarKernel), with no other path out
# of the outer node. In steady state all of the applied heat must flow inward through the
# fluid-side convection (PipeInnerWallTemperatureScalarKernel, Dittus-Boelter) and radial
# conduction resistances in series, which is the series circuit solved algebraically below
# for Tin_analytical/Tout_analytical using fluid properties fixed at Tf1 (mu1, cp1, k1).
#
# As in pipewall_ambient.i, the numeric solution evaluates water properties locally at the
# wall node's own temperature rather than at the fixed Tf1, so its Dittus-Boelter h differs
# slightly from the analytical one; here the wall runs about 1-2 K above Tf1 (heat flows
# into the fluid), smaller than the ~0.3 K excursion in the ambient case, so the expected
# relative mismatch is correspondingly smaller (~1e-4 in the gold file) but is still of the
# same property-evaluation origin, not a convergence or solve-tolerance effect; end_time =
# 100 s is well beyond this system's thermal time constants.

length = 1.0

Di = 0.05
Do = 0.06
Dm = '${fparse ( Di + Do ) / 2}'

k_wall = 15.0
cp_wall = 500.0
rho_wall = 8000.0

Pin = 101300
m1 = 1.0
Tf1 = 293.15
q_flux = 2000.0

# Representative water properties near Tf1, used only to build the independent
# analytical steady-state cross-check below (same constants as the other
# IncompressibleVerification heat tests, e.g. ADheat.i)
mu1 = 0.001002
cp1 = 4185.0
k1 = 0.598

flow_area1 = '${fparse pi / 4 * Di^2}'
wetted_perimeter1 = '${fparse pi * Di}'
Dh1 = '${fparse 4 * flow_area1 / wetted_perimeter1}'
area_in = '${fparse pi / 4 * ( Dm^2 - Di^2 )}'
area_out = '${fparse pi / 4 * ( Do^2 - Dm^2 )}'
interface_perimeter = '${fparse pi * Dm}'
interface_thickness = '${fparse ( Do - Di ) / 2}'
heated_perimeter = '${fparse pi * Do}'

# Independent analytical steady state: a series thermal-resistance circuit
# Tf1 --[fluid convection]-- Tin_wall --[radial conduction]-- Tout_wall, with a
# prescribed heat flux injected directly at the outer node and no other path out of
# the outer node, so in steady state all of the applied heat flows inward through the
# fluid-side convection and radial conduction resistances in series.
Re1 = '${fparse m1 / flow_area1 * Dh1 / mu1}'
Pr1 = '${fparse mu1 * cp1 / k1}'
h1 = '${fparse 0.023 * Re1^0.8 * Pr1^0.4 * k1 / Dh1}'
G1 = '${fparse h1 * wetted_perimeter1}'
G2 = '${fparse k_wall * interface_perimeter / interface_thickness}'
Qflux = '${fparse q_flux * heated_perimeter}'
Tin_analytical = '${fparse Tf1 + Qflux / G1}'
Tout_analytical = '${fparse Tin_analytical + Qflux / G2}'

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
    initial_condition = ${Tf1}
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
    type = PipeOuterWallHeatFluxScalarKernel
    variable = Tout_wall
    inner_wall_temperature = Tin_wall
    upstream_wall_temperature = Tout_wall
    downstream_wall_temperature = Tout_wall
    sp = wall_sp
    heat_flux = ${q_flux}
    heated_perimeter = ${heated_perimeter}
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
