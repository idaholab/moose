# Tests the HeatRateConvection post-processor.

L = 3.0
thickness = 0.1
depth = 5.0
S = ${fparse L * depth}

Q = 5000
T = 300
T_ambient = 350
htc = ${fparse Q / (S * (T_ambient - T))}

[SolidProperties]
  [region1-mat]
    type = ThermalFunctionSolidProperties
    k = 1
    cp = 1
    rho = 1
  []
[]

[Components]
  [heat_structure]
    type = HeatStructurePlate

    position = '1 2 3'
    orientation = '1 1 1'
    length = ${L}
    n_elems = 50

    depth = ${depth}

    names = 'region1'
    solid_properties = 'region1-mat'
    solid_properties_T_ref = '300'
    widths = '${thickness}'
    n_part_elems = '5'

    initial_T = ${T}
  []
[]

[Postprocessors]
  [Q_pp]
    type = HeatRateConvection
    boundary = heat_structure:outer
    htc = ${htc}
    T = T_solid
    T_ambient = ${T_ambient}
    scale = ${depth}
    execute_on = 'initial'
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient
  num_steps = 0
[]

[Outputs]
  file_base = 'heat_rate_convection'
  [csv]
    type = CSV
    precision = 15
    execute_on = 'initial'
  []
[]
