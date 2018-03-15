[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  uniform_refine = 2
[]

[Variables]
  [./u]
  [../]
[]

[AuxVariables]
  [./shared]
    family = SCALAR
    initial_condition = 2
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
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

[Postprocessors]
  [./source_value]
    type = ScalarVariable
    variable = shared
    execute_on = 'initial timestep_end'
  [../]
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  hide = shared
  exodus = true
[]

[DiracKernels]
  [./source_0]
    variable = u
    shared = shared
    type = ReportingConstantSource
    point = '0.2 0.2'
  [../]
  [./source_1]
    point = '0.8 0.8'
    factor = 2
    variable = u
    shared = shared
    type = ReportingConstantSource
  [../]
[]
