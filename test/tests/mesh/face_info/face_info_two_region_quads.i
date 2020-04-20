[Mesh]
  [./generated]
    type = CartesianMeshGenerator
    dim = 2
    dx = '1 1'
    dy = '1 1'
    subdomain_id = '1 2 2 2'
  [../]
[]

[Problem]
  kernel_coverage_check = false
[]

[Variables]
  [./u]
    family = MONOMIAL
    order = CONSTANT
    block = 1
  [../]

  [./v]
    family = MONOMIAL
    order = CONSTANT
    block = 2
  [../]

  [./w]
    family = MONOMIAL
    order = CONSTANT
  [../]

  [_trigger_fv_on]
    fv = true
    family = MONOMIAL
    order = CONSTANT
  []
[]

[VectorPostprocessors]
  [./face_info_1]
    type = TestFaceInfo
    vars = 'u'
  [../]

  [./face_info_2]
    type = TestFaceInfo
    vars = 'v'
  [../]

  [./face_info_3]
    type = TestFaceInfo
    vars = 'w'
  [../]
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
