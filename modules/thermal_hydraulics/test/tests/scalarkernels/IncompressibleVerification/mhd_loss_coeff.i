len1 = 1.0
R = 0.025
min = 1.0
Pin = 101300
dP = -10130
Tin = 700
rho = 10545.35
mu = 0.00209528
alpha = 0.0
epsilon = 0.0001
forms = 0.0
pump = 0.0
gravity = 0.0
area1 = '${fparse pi* ${R}^2}'
peri1 = ${fparse pi*R*2}
B = 1
sigma_lead = 4.8E+06
sigma_steel = 1.0E+06
t_w = 0.001

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[Variables]
  [m1]
    family = SCALAR
    initial_condition = ${min}
  []
  [dPc]
    family = SCALAR
    initial_condition = ${dP}
  []
  [T1]
    family = SCALAR
    initial_condition = ${Tin}
  []
[]

[FluidProperties]
  [lead]
    type = LeadFluidProperties
  []
[]

[FunctorMaterials]
  [k_mhd]
    type = ADMHDFormLossFunctorMaterial
    K_name = KMHD
    temperature = T1
    mass_flow_rate = m1
    inlet_temperature = T1
    outlet_temperature = T1
    area = ${area1}
    field_strength = ${B}
    fp = lead
    perimeter = ${peri1}
    reference_pressure = ${Pin}
    sigma_f = ${sigma_lead}
    sigma_w = ${sigma_steel}
    wall_thickness = ${t_w}
    length = ${len1}
  []
[]

[ScalarKernels]
  [pipe1_mom]
    type = IncompressibleMomentumSPScalarKernel
    variable = 'm1'
    reference_pressure_drop = 'dPc'
    temperatures = 'T1'
    reference_pressure = ${Pin}
    fp = 'lead'
    areas = '${area1}'
    perimeters = '${fparse 2*3.14159* ${R}}'
    lengths = '${len1}'
    alphas = '${alpha}'
    forms_losses = KMHD
    pump_pressures = '${pump}'
    roughnesses = '${epsilon}'
    g = ${gravity}
    is_implicit = True
  []
  [pipe1_DT]
    type = ODETimeDerivative
    variable = 'm1'
  []
  [temp]
    type = ParsedODEKernel
    expression = 'T1 - ${Tin}'
    variable = T1
  []
  [dPk]
    type = ParsedODEKernel
    expression = 'dPc - ${dP}'
    variable = dPc
  []
[]

[Postprocessors]
  [mdot]
    type = ScalarVariable
    variable = m1
    execute_on = 'TIMESTEP_END'
  []
  [Re]
    type = ParsedPostprocessor
    expression = 'mdot / ${area1} * 2 * ${R} / ${mu}'
    pp_names = 'mdot'
    execute_on = 'TIMESTEP_END'
  []
  [f]
    type = ParsedPostprocessor
    expression = '0.25 / ((log10(${epsilon} / 2 / 3.7 / ${R} + 5.74 / Re ^ 0.9 ))^2)'
    pp_names = 'Re'
    execute_on = 'TIMESTEP_END'
  []
  [phi]
    type = ParsedPostprocessor
    expression = '2 * ${sigma_steel} * ${t_w} / ${sigma_lead} / 2 / ${R}'
    execute_on = 'TIMESTEP_END'
  []
  [hartmann]
    type = ParsedPostprocessor
    expression = '${B} * ${R} * sqrt(${sigma_lead} / ${mu})'
    execute_on = 'TIMESTEP_END'
  []
  [Kmhd]
    type = ParsedPostprocessor
    expression = '2 * ${len1} * ${sigma_lead} * ${B}^2 * ${area1} / mdot * ( 1 / (hartmann - 1) + phi / (1 + phi))'
    pp_names = 'mdot hartmann phi'
    execute_on = 'TIMESTEP_END'
  []
  [analytical_mdot]
    type = ParsedPostprocessor
    expression = 'sqrt(-(${dP} + ${rho} * ${gravity} * ${len1} * sin(${alpha}) - ${pump}) / (f * ${len1} / 4 / ${R} / ${rho} / ${area1}^2 + (${forms} + Kmhd) / 2 / ${rho} / ${area1}^2))'
    pp_names = 'f Kmhd'
    execute_on = 'TIMESTEP_END'
  []
  [relative_error]
    type = ParsedPostprocessor
    expression = 'abs((analytical_mdot - mdot)/analytical_mdot)'
    pp_names = 'analytical_mdot mdot'
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
  nl_abs_tol = 1e-09
  l_tol = 1e-07
[]

[Outputs]
  perf_graph = true
  [out]
    type = CSV
    execute_on = 'FINAL'
  []
[]
