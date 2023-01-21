[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  xmin = 0.0
  xmax = 10.0
  ymin = 0.0
  ymax = 10.0
  elem_type = QUAD4
[]

[Functions]
  [./mask_func]
    type = ParsedFunction
    expression = 'r:=sqrt((x-5)^2+(y-5)^2); if (r<3, 1.0, 0.0)'
  [../]
[]

[Variables]
  [./c]
    order = FIRST
    family = LAGRANGE
    initial_condition = 0.0
  [../]
[]

[Kernels]
  [./time]
    type = TimeDerivative
    variable = c
  [../]

  [./conserved_langevin]
    type = ConservedLangevinNoise
    amplitude = 0.5
    variable = c
    noise = normal_masked_noise
  []
[]

[BCs]
  [./Periodic]
    [./all]
      variable = c
      auto_direction = 'x y'
    [../]
  [../]
[]

[Materials]
  [./mask_material]
    type = GenericFunctionMaterial
    prop_names  = 'mask_prop'
    prop_values = 'mask_func'
  [../]
[]

[UserObjects]
  [./normal_masked_noise]
    type = ConservedMaskedNormalNoise
    mask = mask_prop
  [../]
[]

[Postprocessors]
  [./total_c]
    type = ElementIntegralVariablePostprocessor
    variable = c
  [../]
[]

[Executioner]
  type = Transient
  scheme = 'BDF2'

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

  l_max_its = 30
  l_tol = 1.0e-3

  nl_max_its = 30
  nl_rel_tol = 1.0e-8
  nl_abs_tol = 1.0e-10

  dt = 10.0
  num_steps = 4
[]

[Outputs]
  file_base = normal_masked
  [./csv]
    type = CSV
  [../]
[]
