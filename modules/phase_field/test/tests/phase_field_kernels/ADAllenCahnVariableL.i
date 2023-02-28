#
# Test the forward automatic differentiation Allen-Cahn Bulk kernel with a
# spatially varying mobility
#

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  xmax = 12
  ymax = 12
  elem_type = QUAD4
[]

[AuxVariables]
  [./chi]
    [./InitialCondition]
      type = FunctionIC
      function = 'x/24+0.5'
    [../]
  [../]
[]

[Variables]
  [./eta]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
      type = SmoothCircleIC
      x1 = 0.0
      y1 = 0.0
      radius = 6.0
      invalue = 0.9
      outvalue = 0.1
      int_width = 3.0
    [../]
  [../]
[]

[Kernels]
  [./detadt]
    type = ADTimeDerivative
    variable = eta
  [../]

  [./ACBulk]
    type = ADAllenCahn
    variable = eta
    f_name = F
  [../]

  [./ACInterface]
    type = ADACInterface
    variable = eta
    kappa_name = 1
    variable_L = true
    coupled_variables = chi
  [../]
[]

[Materials]
  [./L]
    type = ADTestDerivativeFunction
    function = F2
    f_name = L
    op = 'eta chi'
  [../]

  [./free_energy]
    type = ADTestDerivativeFunction
    function = F1
    f_name = F
    op = 'eta'
  [../]
[]

[Executioner]
  type = Transient
  scheme = 'bdf2'
  solve_type = 'NEWTON'
  num_steps = 2
  dt = 1
[]

[Outputs]
  exodus = true
[]
