#
# Test the parsed function free enery Allen-Cahn Bulk kernel
#

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 10
  xmin = 0
  xmax = 1
[]

[AuxVariables]
  [./eta]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
      type = FunctionIC
      function = x
    [../]
  [../]
[]

[Materials]
  [./consts]
    type = ParsedMaterial
    coupled_variables  = 'eta'
    expression ='(eta-0.5)^2'
    outputs = exodus
  [../]
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
[]
