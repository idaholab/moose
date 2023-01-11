[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  nx = 10
  ny = 10
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[AuxVariables]
  [./coupled_bc_var]
  [../]
[]

[ICs]
  [./coupled_bc_var]
    type = FunctionIC
    variable = coupled_bc_var
    function = set_coupled_bc_var
  [../]
[]

[Functions]
  [./set_coupled_bc_var]
    type = ParsedFunction
    expression = 'y - 0.5'
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = 3
    value = 0
  [../]

  [./right]
    type = CoupledVarNeumannBC
    variable = u
    boundary = 1
    v = coupled_bc_var
  [../]
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
