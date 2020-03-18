[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10 
  ny = 10
[]

[Variables]
  [v]
    family = MONOMIAL
    order = CONSTANT
    fv = true
  []
[]

#[ICs]
#  [left]
#      type = ConstantIC
#      variable = v
#      value = 7
#      block = '1'
#  []
#[]

[Kernels]
  [./time]
    type = TimeDerivative
    variable = v
  [../]
[]

[FVKernels]
  [diff]
    type = FVDiffusion
    variable = v
    coeff = coeff
  []
[]

[FVBCs]
  [left]
    type = FVDirichletBC
    variable = v
    boundary = left
    value = 7
  []
  [right]
    type = FVDirichletBC
    variable = v
    boundary = right
    value = 42
  []
[]

[Materials]
  [diff]
    type = GenericConstantMaterial
    prop_names = 'coeff'
    prop_values = '1'
  []
[]

[Problem]
  kernel_coverage_check = off
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  num_steps = 20
  dt = 0.1
[]

[Outputs]
  exodus = true
[]
