[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
  allow_renumbering = false
[]

[Materials]
  [props]
    type = ADGenericConstantMaterial
    prop_names = 'rho vel mu D_h'
    prop_values = '1000 0.1 0.001 0.15'
  []

  [fD_material]
    type = ADWallFrictionChurchillMaterial
    rho = rho
    vel = vel
    D_h = D_h
    mu = mu
    f_D = 'f_D'
    roughness = 0.5
  []
[]

[Postprocessors]
  [fD]
    type = ADElementAverageMaterialProperty
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
