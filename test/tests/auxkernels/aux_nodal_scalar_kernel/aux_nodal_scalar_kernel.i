[Mesh]
  type = GeneratedMesh
  dim = 1
  xmin = 0
  xmax = 1
  nx = 10
[]

[Variables]
  [./u]
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
    boundary = 0
    value = 1
  [../]
  
  [./right]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 2
  [../]
[]

[AuxVariables]
  [./bc_sum]
    family = SCALAR
    order = FIRST
  [../]
[]

[AuxScalarKernels]
  [./sk]
    type = SumNodalValuesAux
    variable = bc_sum
    nodes = '0 10'
    sum_var = u
  [../]
[]

[Postprocessors]
  [./sum]
    type = PrintScalarVariable
    variable = bc_sum
  [../]
[]

[Executioner]
  type = Steady
[]

[Output]
  exodus = true
  output_initial = true
[]