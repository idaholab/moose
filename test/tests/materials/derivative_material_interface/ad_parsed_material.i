#
# Test the parsed function free enery Allen-Cahn Bulk kernel
#

[Mesh]
  type = GeneratedMesh
  dim = 2
[]

[Variables]
  [./eta]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
      type = FunctionIC
      function = x
    [../]
  [../]
[]

[Kernels]
  [./diff]
    type = ADMatDiffusion
    variable = eta
    diffusivity = F
  [../]
  [./dt]
    type = TimeDerivative
    variable = eta
  [../]
[]

[Materials]
  [./consts]
    type = ADParsedMaterial
    coupled_variables  = 'eta'
    expression ='(eta-0.5)^2'
    outputs = exodus
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 1
  dt = 0.1
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
[]
