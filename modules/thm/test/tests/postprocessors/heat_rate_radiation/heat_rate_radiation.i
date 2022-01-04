# Tests the HeatRateRadiation post-processor.

L = 3.0
thickness = 0.1
depth = 5.0
S = ${fparse L * depth}

Q = 5000
T = 300
T_ambient = 350
sigma = 5.670367e-8
emissivity = ${fparse Q / (S * sigma * (T_ambient^4 - T^4))}

[HeatStructureMaterials]
  [region1-mat]
    type = SolidMaterialProperties
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
    materials = 'region1-mat'
    widths = '${thickness}'
    n_part_elems = '5'

    initial_T = ${T}
  []
[]

[Postprocessors]
  [Q_pp]
    type = HeatRateRadiation
    boundary = heat_structure:outer
    T = T_solid
    T_ambient = ${T_ambient}
    emissivity = ${emissivity}
    stefan_boltzmann_constant = ${sigma}
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
  file_base = 'heat_rate_radiation'
  [csv]
    type = CSV
    precision = 15
    execute_on = 'initial'
  []
[]
