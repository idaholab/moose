# apply a half-cubic heat sink flux
[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1
  ny = 1
  nz = 2
  xmin = -1
  xmax = 1
  ymin = -1
  ymax = 1
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[Variables]
  [temp]
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = temp
    number_fluid_phases = 0
    number_fluid_components = 0
  []
[]

[ICs]
  [temp]
    type = RandomIC
    variable = temp
    min = -1
    max = 0
  []
[]

[Kernels]
  [dummy_temp]
    type = TimeDerivative
    variable = temp
  []
[]

[Materials]
  [temperature]
    type = PorousFlowTemperature
    temperature = temp
  []
[]

[BCs]
  [flux_w]
    type = PorousFlowHalfCubicSink
    boundary = 'left'
    center = 0.1
    cutoff = -1.1
    max = 2.2
    variable = temp
    flux_function = 'x*y'
  []
  [flux_g]
    type = PorousFlowHalfCubicSink
    boundary = 'top left front'
    center = 0.5
    cutoff = -1.1
    max = -2.2
    variable = temp
    flux_function = '-x*y'
  []
  [flux_1]
    type = PorousFlowHalfCubicSink
    boundary = 'right'
    center = -0.1
    cutoff = -1.1
    max = 1.2
    variable = temp
    flux_function = '-1.1*x*y'
  []
  [flux_2]
    type = PorousFlowHalfCubicSink
    boundary = 'bottom'
    center = 3.2
    cutoff = -1.1
    max = 1.2
    variable = temp
    flux_function = '0.5*x*y'
  []
[]


[Preconditioning]
  [check]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it -snes_type'
    petsc_options_value = 'bcgs bjacobi 1E-15 1E-10 10000 test'
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  dt = 1
  end_time = 1
[]

[Outputs]
  file_base = hcs02
[]
