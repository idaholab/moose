[HeatStructureMaterials]
  [a]
    type = SolidMaterialProperties
    rho = 1
    cp = 1
    k = 1
  []
[]

[Components]
  [warn]
    type = LogWarningComponent
  []

  [hs]
    type = HeatStructureCylindrical
    position = '0 0 0'
    orientation = '1 0 0'
    length = 1
    names = '0'
    widths = '0.1'
    materials = 'a'
    n_elems = 1
    n_part_elems = 1
    initial_T = 300
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient
  num_steps = 1
[]
