# Tests the HeatRateHeatFlux post-processor.

thickness = 0.02
depth = 2.0

L = 3.0
S = ${fparse depth * L}

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
    type = HeatStructurePlate

    position = '1 2 3'
    orientation = '1 1 1'
    length = ${L}
    n_elems = 50

    names = 'region1'
    materials = 'region1-mat'
    widths = '${thickness}'
    n_part_elems = '5'

    depth = ${depth}

    initial_T = 300
  []
[]

[Postprocessors]
  [Q_pp]
    type = HeatRateHeatFlux
    boundary = heat_structure:outer
    q = ${q}
    scale = ${depth}
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
