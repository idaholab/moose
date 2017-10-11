[Mesh]
  type = GeneratedMesh
  dim = 2
  xmax = 2
  nx = 20
  ny = 10
[]

[Variables]
  [./not_u]
  [../]
[]

[AuxVariables]
  [./u]
    family = MONOMIAL
    order = FIRST
  [../]
  [./grad_u_x]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[ICs]
  [./u]
    type = FunctionIC
    variable = u
    function = 'if(x>0.5,if(x<1.5,2*x,3),0)'
  [../]
[]

[AuxKernels]
  [./grad_u_x_aux]
    type = VariableGradientComponent
    variable = grad_u_x
    component = x
    gradient_variable = u
    execute_on = initial
  [../]
[]

[Problem]
  type = FEProblem
  solve = false
  kernel_coverage_check = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
