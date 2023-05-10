# Tests the HeatRateHeatFluxRZ post-processor.

R_o = 0.2
thickness = 0.05
R_i = ${fparse R_o - thickness}

L = 3.0
S = ${fparse 2 * pi * R_o * L}

Q = 5000
q = ${fparse Q / S}

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
    type = HeatStructureCylindrical

    position = '1 2 3'
    orientation = '1 1 1'
    inner_radius = ${R_i}
    length = ${L}
    n_elems = 50

    names = 'region1'
    materials = 'region1-mat'
    widths = '${thickness}'
    n_part_elems = '5'

    initial_T = 300
  []
[]

[Postprocessors]
  [Q_pp]
    type = HeatRateHeatFluxRZ
    boundary = heat_structure:outer
    axis_point = '1 2 3'
    axis_dir = '1 1 1'
    q = ${q}
    execute_on = 'INITIAL'
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
  csv = true
  execute_on = 'INITIAL'
[]
