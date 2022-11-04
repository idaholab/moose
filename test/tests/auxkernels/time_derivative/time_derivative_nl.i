[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 10
  xmin = -5.0
  xmax =  5.0
[]

[Variables]
  [./c]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables]
  [./c_dot]
    order = FIRST
    family = LAGRANGE
  [../]
  [./c_dot_elem]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  [./coupled_dot]
    type = DotCouplingAux
    variable = c_dot
    v = c
  [../]
  [./coupled_dot_elem]
    type = DotCouplingAux
    variable = c_dot_elem
    v = c
  [../]
[]

[ICs]
  [./centered_gauss_func]
    type = FunctionIC
    variable = c
    function = gaussian_1d
  [../]
[]

[Functions]
  [./gaussian_1d]
    type = ParsedFunction
    expression = exp(-x*x/2.0/1.0/1.0)
  [../]
[]

[Kernels]
  [./dot]
    type = TimeDerivative
    variable = c
  [../]
  [./diff]
    type = Diffusion
    variable = c
  [../]
[]

[BCs]
  [./Periodic]
    [./auto]
      variable = c
      auto_direction = 'x'
    [../]
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'
  dt = 0.1
  num_steps = 5
[]


[Outputs]
  exodus = true
  #
[]
