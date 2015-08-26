[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
  elem_type = QUAD4
  uniform_refine = 4
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
    type = FunctionValuePostprocessor
    function = '2*(x+y)'
    point = '0.5 0.5 0'
  [../]
  [./other_point_test_object]
    type = FunctionValuePostprocessor
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
    parameter = 'Postprocessor::point'
    execute_on = 'initial'
  [../]
[]
