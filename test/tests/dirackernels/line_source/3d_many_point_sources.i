[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 10
  ny = 10
  nz = 10
  xmin = -2
  xmax =  2
  ymin = -2
  ymax =  2
  ymin = -2
  ymax =  2
[]

[Variables]
  active = 'u'
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[DiracKernels]
  [./ps1]
    type = ConstantPointSource
    variable = u
    point = '0.996917333742 0.0784590956185 0.0499999999302'
    value = 0.000465523970638
  [../]

  [./ps2]
    type = ConstantPointSource
    variable = u
    point = '0.972369920432 0.233445363714 0.149999999907'
    value = 0.00418971574225
  [../]

  [./ps3]
    type = ConstantPointSource
    variable = u
    point = '0.923879532616 0.382683432112 0.249999999825'
    value = 0.0116380992822
  [../]

  [./ps4]
    type = ConstantPointSource
    variable = u
    point = '0.852640164564 0.522498564373 0.349999999744'
    value = 0.0228106745916
  [../]

  [./ps5]
    type = ConstantPointSource
    variable = u
    point = '0.760405965885 0.649448047996 0.449999999721'
    value = 0.0377074416802
  [../]

  [./ps6]
    type = ConstantPointSource
    variable = u
    point = '0.649448048692 0.760405965291 0.549999999697'
    value = 0.0563284005426
  [../]

  [./ps7]
    type = ConstantPointSource
    variable = u
    point = '0.522498565153 0.852640164087 0.649999999674'
    value = 0.0786735511788
  [../]

  [./ps8]
    type = ConstantPointSource
    variable = u
    point = '0.382683432703 0.923879532371 0.749999999767'
    value = 0.104742893621
  [../]

  [./ps9]
    type = ConstantPointSource
    variable = u
    point = '0.233445364069 0.972369920346 0.84999999986'
    value = 0.134536427846
  [../]

  [./ps10]
    type = ConstantPointSource
    variable = u
    point = '0.0784590958008 0.996917333727 0.949999999953'
    value = 0.168054153854
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 0
  [../]
[]

[Postprocessors]
  [./pt_v]
    type = PointValue
    variable = u
    point = '0 0 0'
  [../]
[]

[Executioner]
  type = Steady

  # Preconditioned JFNK (default)
  solve_type = 'PJFNK'

[]

[Outputs]
  exodus = true
[]
