[Mesh]
  type = FileMesh
  file = rectangle.e
  dim = 2
  uniform_refine = 2
[]

[Variables]
  [./u]
  [../]
[]

[AuxVariables]
  [./real_property]
    family = MONOMIAL
    order = CONSTANT
  [../]
[]

[AuxKernels]
  [./real_property]
    type = MaterialRealAux
    variable = real_property
    property = real_property
    boundary = '1 2'
  [../]
[]

[Kernels]
  [./diff]
    type = CoefDiffusion
    variable = u
    coef = 1
  [../]
  [./time]
    type = TimeDerivative
    variable = u
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = 2
    value = 1
  [../]
[]

[Materials]
  [./boundary_1]
    type = OutputTestMaterial
    boundary = 1
    real_factor = 2
    variable = u
  [../]
  [./boundary_2]
    type = OutputTestMaterial
    boundary = 2
    real_factor = 2
    variable = u
 [../]
[]

[Executioner]
  type = Transient
  num_steps = 2
  dt = 0.1
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
