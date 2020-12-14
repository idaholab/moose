[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
  allow_renumbering = false
[]


[Variables]
  [rhoA]
  []
  [rhouA]
  []
  [rhoEA]
  []
[]

[Materials]
  [props]
    type = GenericConstantMaterial
    prop_names = 'rho vel D_h mu '
    prop_values = '1000 0.1 0.15 0.001'
  []

  [fD_material]
    type = WallFrictionChurchillMaterial
    rho = rho
    vel = vel
    D_h = D_h
    mu = mu
    f_D = 'f_D'
    rhoA = rhoA
    rhouA = rhouA
    rhoEA = rhoEA
    roughness = 0.5
  []
[]

[Postprocessors]
  [fD]
    type = ElementAverageMaterialProperty
    mat_prop = f_D
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Outputs]
  csv = true
  execute_on = 'FINAL'
[]
