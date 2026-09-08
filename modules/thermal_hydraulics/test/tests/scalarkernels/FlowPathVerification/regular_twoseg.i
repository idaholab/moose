

length1 = 1.0
length2 = 2.0
R1 = 0.025
R2 = 0.05
min = 1.0
Pin = 101300
dP = -10130
Tin = 293.15
rho = 998.2
mu = 0.001002
alpha1 = 0.0
alpha2 = ${fparse 3.14159/2}
epsilon1 = 0.0001
epsilon2 = 0.0002
forms1 = 0.0
forms2 = 1.0
pump1 = ${fparse - ${dP}}
pump2 = 0.0
gravity = 9.81
area1 = ${fparse 3.14159* ${R1}^2}
area2 = ${fparse 3.14159* ${R2}^2}

[Mesh]
  type = GeneratedMesh
  dim = 1
  xmin = 0
  xmax = ${length1}
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
  [water]
    type = Water97FluidProperties
  []
[]

[ScalarKernels]
    [pipe1_mom]
        type = FlowPathMomentumSCSPScalarKernel
        variable = 'm1'
        reference_pressure_drop = 'dPc'
        temperatures = 'T1 T1'
        reference_pressure = ${Pin}
        fp = 'water'
        areas = '${area1} ${area2}'
        perimeters = '${fparse 2*3.14159* ${R1}} ${fparse 2*3.14159* ${R2}}'
        lengths = '${length1} ${length2}'
        alphas = '${alpha1} ${alpha2}'
        forms_losses = '${forms1} ${forms2}'
        pump_pressures = '${pump1} ${pump2}'
        roughnesses = '${epsilon1} ${epsilon2}'
        g = ${gravity}
        is_implicit = True
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
  [Re1]
    type = ParsedPostprocessor
    expression = 'mdot / ${area1} * 2 * ${R1} / ${mu}'
    pp_names = 'mdot'
    execute_on = 'TIMESTEP_END'
  []
  [Re2]
    type = ParsedPostprocessor
    expression = 'mdot / ${area2} * 2 * ${R2} / ${mu}'
    pp_names = 'mdot'
    execute_on = 'TIMESTEP_END'
  []
  [f1]
    type = ParsedPostprocessor
    expression = '0.25 / ((log10(${epsilon1} / 2 / 3.7 / ${R1} + 5.74 / Re1 ^ 0.9 ))^2)'
    pp_names = 'Re1'
    execute_on = 'TIMESTEP_END'
  []
  [f2]
    type = ParsedPostprocessor
    expression = '0.25 / ((log10(${epsilon2} / 2 / 3.7 / ${R2} + 5.74 / Re2 ^ 0.9 ))^2)'
    pp_names = 'Re2'
    execute_on = 'TIMESTEP_END'
  []
  [analytical_mdot]
    type = ParsedPostprocessor
    expression = 'sqrt(-(${dP} + ${rho} * ${gravity} * ${length1} * sin(${alpha1})
                          + ${rho} * ${gravity} * ${length2} * sin(${alpha2})
                          - ${pump1} - ${pump2})
                          / (f1 * ${length1} / 4 / ${R1} / ${rho} / ${area1}^2
                          + f2 * ${length2} / 4 / ${R2} / ${rho} / ${area2}^2
                          + ${forms1} / 2 / ${rho} / ${area1}^2
                          + ${forms2} / 2 / ${rho} / ${area2}^2))'
    pp_names = 'f1 f2'
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
    file_base = 'friction_only'
  []
[]
