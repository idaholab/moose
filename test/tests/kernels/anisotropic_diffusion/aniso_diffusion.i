[Mesh]
  file = mixed_block.e
  uniform_refine=3
[]

[Functions]
  [./top_bc]
    type = ParsedFunction
    expression = 'x'
  [../]
[]

[Variables]
  active = 'u'

  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./diff]
    type = AnisotropicDiffusion
    variable = u
    tensor_coeff = '2 0 0
                    0 4 0
        0 0 0'
  [../]
[]

[BCs]
  active = 'lower_left top'

  [./lower_left]
    type = DirichletBC
    variable = u
    boundary = '1 4'
    value = 1
  [../]

  [./top]
    type = FunctionNeumannBC
    variable = u
    boundary = 3
    function = top_bc
  [../]

  [./right]
    type = NeumannBC
    variable = u
    boundary = 2
  [../]
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'
[]

[Outputs]
  exodus = true
[]
