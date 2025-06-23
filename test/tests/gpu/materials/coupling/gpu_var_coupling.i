# The diffusion coefficient is given as the variable itself, which makes it a non-linear problem.

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [./u]
    initial_condition = 1
  [../]
[]

[AuxVariables]
  [./v]
    initial_condition = 1
  [../]
[]

[GPUKernels]
  [./diff]
    type = GPUMatDiffusionTest
    variable = u
    prop_name = diffusion
  [../]
[]

[GPUBCs]
  [./left]
    type = GPUDirichletBC
    variable = u
    boundary = left
    value = 0
  [../]
  [./right]
    type = GPUDirichletBC
    variable = u
    boundary = right
    value = 1
  [../]
[]

[GPUMaterials]
  [./coupling_u]
    type = GPUVarCouplingMaterial
    block = 0
    var = u
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 1
  dt = 1

  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
