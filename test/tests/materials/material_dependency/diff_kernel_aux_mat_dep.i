[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 1
  ny = 1
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [./diff]
    type = MatDiffusionTest
    variable = u
    prop_name = 'diff'
  [../]
[]

[AuxKernels]
  [./error]
    type = ElementLpNormAux
    variable = error
    coupled_variable = u
  [../]
[]

[AuxVariables]
  [./error]
    family = MONOMIAL
    order = FIRST
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
    value = 1
  [../]
[]

[Materials]
  [./call_me_mat]
    type = IncrementMaterial
    prop_names = 'diff'
    prop_values = '1'
    block = 0
    outputs = exodus
    output_properties = 'mat_prop'
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
