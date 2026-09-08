

length1 = 1.0
length2 = 1.0
length3 = 1.0
R1 = 0.025
R2 = 0.05
min = 1.0
Pin = 101300
dP = -10130
Tin = 293.15
Tout = 343.15
mu = 0.001002
cp = 4185.0
k = 0.598
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
    [T0]
        family = SCALAR
        initial_condition = ${Tin}
    []
    [T1]
        family = SCALAR
        initial_condition = ${Tin}
    []
    [Tw1]
        family = SCALAR
        initial_condition = ${Tout}
    []
    [T2]
        family = SCALAR
        initial_condition = ${Tout}
    []
    [Tw2]
        family = SCALAR
        initial_condition = ${Tout}
    []
    [T3]
        family = SCALAR
        initial_condition = ${Tout}
    []
    [Tw3]
        family = SCALAR
        initial_condition = ${Tout}
    []
    [T4]
        family = SCALAR
        initial_condition = ${Tout}
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
        temperatures = 'T1 T2 T3'
        reference_pressure = ${Pin}
        fp = 'water'
        areas = '${area1} ${area2} ${area2}'
        perimeters = '${fparse 2*3.14159* ${R1}} ${fparse 2*3.14159* ${R2}} ${fparse 2*3.14159* ${R2}}'
        lengths = '${length1} ${length2} ${length2}'
        alphas = '${alpha1} ${alpha2} ${alpha2}'
        forms_losses = '${forms1} ${forms2} ${forms2}'
        pump_pressures = '${pump1} ${pump2} ${pump2}'
        roughnesses = '${epsilon1} ${epsilon2} ${epsilon2}'
        g = ${gravity}
        is_implicit = True
    []
    [temp0]
      type = ParsedODEKernel
      expression = 'T0 - ${Tin}'
      variable = T0
    []
    [temp1]
      type = FlowPathEnergySCSPScalarKernel
      mass_flow_rate = 'm1'
      inlet_temperature = 'T0'
      outlet_temperature = 'T2'
      wall_temperature = 'Tw1'
      area = ${area1}
      fp = water
      length = ${length1}
      perimeter = ${fparse 2*3.14159* ${R1}}
      reference_pressure = ${Pin}
      variable = T1
      is_implicit = True
    []
    [temp2]
      type = FlowPathEnergySCSPScalarKernel
      mass_flow_rate = 'm1'
      inlet_temperature = 'T1'
      outlet_temperature = 'T3'
      wall_temperature = 'Tw2'
      area = ${area2}
      fp = water
      length = ${length2}
      perimeter = ${fparse 2*3.14159* ${R2}}
      reference_pressure = ${Pin}
      variable = T2
      is_implicit = True
    []
    [temp3]
      type = FlowPathEnergySCSPScalarKernel
      mass_flow_rate = 'm1'
      inlet_temperature = 'T2'
      outlet_temperature = 'T4'
      wall_temperature = 'Tw3'
      area = ${area2}
      fp = water
      length = ${length3}
      perimeter = ${fparse 2*3.14159* ${R2}}
      reference_pressure = ${Pin}
      variable = T3
      is_implicit = True
    []
    [temp4]
      type = ParsedODEKernel
      expression = 'T4 - ${Tout}'
      variable = T4
    []
    [walltemp1]
      type = ParsedODEKernel
      expression = 'Tw1 - ${Tout}'
      variable = Tw1
    []
    [walltemp2]
      type = ParsedODEKernel
      expression = 'Tw2 - ${Tout}'
      variable = Tw2
    []
    [walltemp3]
      type = ParsedODEKernel
      expression = 'Tw3 - ${Tout}'
      variable = Tw3
    []
    [dPk]
      type = ParsedODEKernel
      expression = 'dPc - ${dP}'
      variable = dPc
    []
[]

[Postprocessors]
  [Tone]
    type = ScalarVariable
    variable = T1
    execute_on = 'TIMESTEP_END'
  []
  [Ttwo]
    type = ScalarVariable
    variable = T2
    execute_on = 'TIMESTEP_END'
  []
  [Tthree]
    type = ScalarVariable
    variable = T3
    execute_on = 'TIMESTEP_END'
  []
  [Re1]
    type = ParsedPostprocessor
    expression = 'abs(${min}) / ${area1} * 2 * ${R1} / ${mu}'
    execute_on = 'TIMESTEP_END'
  []
  [Re2]
    type = ParsedPostprocessor
    expression = 'abs(${min}) / ${area2} * 2 * ${R2} / ${mu}'
    execute_on = 'TIMESTEP_END'
  []
  [Pr]
    type = ParsedPostprocessor
    expression = '${cp} * ${mu} / ${k}'
    execute_on = 'TIMESTEP_END'
  []
  [h1]
    type = ParsedPostprocessor
    expression = '0.023 * Re1^0.8 * Pr^0.4'
    pp_names = 'Re1 Pr'
    execute_on = 'TIMESTEP_END'
  []
  [h2]
    type = ParsedPostprocessor
    expression = '0.023 * Re2^0.8 * Pr^0.4'
    pp_names = 'Re2 Pr'
    execute_on = 'TIMESTEP_END'
  []
  [in1]
    type = ParsedPostprocessor
    expression = '1 / 2 * (1 - abs(${min})/${min}) * Ttwo
                  + 1 / 2 * (1 + abs(${min})/${min}) * ${Tin}'
    pp_names = 'Ttwo'
    execute_on = 'TIMESTEP_END'
  []
  [in2]
    type = ParsedPostprocessor
    expression = '1 / 2 * (1 - abs(${min})/${min}) * Tthree
                  + 1 / 2 * (1 + abs(${min})/${min}) * Tone'
    pp_names = 'Tone Tthree'
    execute_on = 'TIMESTEP_END'
  []
  [in3]
    type = ParsedPostprocessor
    expression = '1 / 2 * (1 - abs(${min})/${min}) * ${Tout}
                  + 1 / 2 * (1 + abs(${min})/${min}) * Ttwo'
    pp_names = 'Ttwo'
    execute_on = 'TIMESTEP_END'
  []
  [q1]
    type = ParsedPostprocessor
    expression = 'h1 * ${fparse 2*3.14159* ${R1}} / 2 * ( 2 * ${Tout} - Tone - in1)'
    pp_names = 'Tone h1 in1'
    execute_on = 'TIMESTEP_END'
  []
  [q2]
    type = ParsedPostprocessor
    expression = 'h2 * ${fparse 2*3.14159* ${R2}} / 2 * ( 2 * ${Tout} - Ttwo - in2)'
    pp_names = 'Ttwo h2 in2'
    execute_on = 'TIMESTEP_END'
  []
  [q3]
    type = ParsedPostprocessor
    expression = 'h2 * ${fparse 2*3.14159* ${R2}} / 2 * ( 2 * ${Tout} - Tthree - in3)'
    pp_names = 'Tthree h2 in3'
    execute_on = 'TIMESTEP_END'
  []
  [analytical_T1]
    type = ParsedPostprocessor
    expression = '1 / abs(${min}) / ${cp} * (q1 * ${length1} - ${min} / 2 * (1 - abs(${min})/${min}) * ${cp} * Ttwo
                  + ${min} / 2 * (1 + abs(${min})/${min}) * ${cp} * ${Tin})'
    pp_names = 'Ttwo q1'
  []
  [analytical_T2]
    type = ParsedPostprocessor
    expression = '1 / abs(${min}) / ${cp} * (q2 * ${length2} - ${min} / 2 * (1 - abs(${min})/${min}) * ${cp} * Tthree
                  + ${min} / 2 * (1 + abs(${min})/${min}) * ${cp} * Tone)'
    pp_names = 'Tone Tthree q2'
  []
  [analytical_T3]
    type = ParsedPostprocessor
    expression = '1 / abs(${min}) / ${cp} * (q3 * ${length3} - ${min} / 2 * (1 - abs(${min})/${min}) * ${cp} * ${Tout}
                  + ${min} / 2 * (1 + abs(${min})/${min}) * ${cp} * Ttwo)'
    pp_names = 'Ttwo q3'
  []
  [relative_error1]
    type = ParsedPostprocessor
    expression = 'abs((analytical_T1 - Tone)/analytical_T1)'
    pp_names = 'analytical_T1 Tone'
    execute_on = 'TIMESTEP_END'
  []
  [relative_error2]
    type = ParsedPostprocessor
    expression = 'abs((analytical_T2 - Ttwo)/analytical_T2)'
    pp_names = 'analytical_T2 Ttwo'
    execute_on = 'TIMESTEP_END'
  []
  [relative_error3]
    type = ParsedPostprocessor
    expression = 'abs((analytical_T3 - Tthree)/analytical_T3)'
    pp_names = 'analytical_T3 Tthree'
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
    file_base = 'heat'
  []
[]
