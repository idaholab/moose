[Mesh]
  type = GeneratedMesh
  dim = 2
  elem_type = QUAD4

  # use odd numbers so points do not fall on element boundaries
  nx = 31
  ny = 31
[]

[Variables]
  [./diffused]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = diffused
  [../]
[]

[DiracKernels]
  [./test_object]
    type = MaterialPointSource
    point = '0.5 0.5 0'
    variable = diffused
  [../]
[]

[BCs]
  [./bottom_diffused]
    type = DirichletBC
    variable = diffused
    boundary = 'bottom'
    value = 2
  [../]

  [./top_diffused]
    type = DirichletBC
    variable = diffused
    boundary = 'top'
    value = 0
  [../]
[]

[Materials]
  [./mat]
    type = GenericConstantMaterial
    prop_names = 'matp'
    prop_values = '1'
    block = 0
  [../]
[]

[Postprocessors]
  [./test_object]
    type = TestControlPointPP
    function = '2*(x+y)'
    point = '0.5 0.5 0'
  [../]
  [./other_point_test_object]
    type = TestControlPointPP
    function = '3*(x+y)'
    point = '0.5 0.5 0'
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
[]

[Controls]
  [./point_control]
    type = TestControl
    test_type = 'point'
    parameter = 'DiracKernels/test_object/point'
    execute_on = 'initial'
  [../]
[]
