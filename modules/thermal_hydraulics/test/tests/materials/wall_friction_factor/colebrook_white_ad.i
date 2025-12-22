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
    prop_values = '1000 1. 0.001 0.1'
  []

  [fD_material]
    type = ADWallFrictionColebrookWhiteMaterial
    rho = rho
    vel = vel
    D_h = D_h
    mu = mu
    f_D = 'f_D'
    roughness = 0.001
  []
[]

[Postprocessors]
  [fD]
    type = ADElementAverageMaterialProperty
    mat_prop = f_D
  []
  [fD_residual]
    type = ParsedPostprocessor
    expression = '(1/sqrt(fD) + 2*log10(eps/(3.7*D_h) + 2.51/(Re*sqrt(fD))))/(1/sqrt(fD))'
    constant_names = 'D_h Re eps'
    constant_expressions = '0.1 1e5 0.001'
    pp_names = 'fD'
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
