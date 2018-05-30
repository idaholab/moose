[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [./u]
  [../]
[]

[AuxVariables]
  [./vpp_0]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./vpp_1]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./vpp_2]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[AuxKernels]
  [./vpp_0]
    type = VectorPostprocessorAux
    variable = vpp_0
    index = 0
    vector = value
    vpp = constant
  [../]
  [./vpp_1]
    type = VectorPostprocessorAux
    variable = vpp_1
    index = 1
    vector = value
    vpp = constant
  [../]
  [./vpp_2]
    type = VectorPostprocessorAux
    variable = vpp_2
    index = 2
    vector = value
    vpp = constant
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

[VectorPostprocessors]
  [./constant]
    type = ConstantVectorPostprocessor
    value = '1.2 3.4 9.6'
    execute_on = initial
  [../]
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
