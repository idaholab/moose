[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = -1
  ymin = -1
  xmax = 1
  ymax = 1
  nx = 2
  ny = 2
  elem_type = QUAD4
[]

[Functions]
  [./fn_exact]
    type = ParsedFunction
    expression = 'x*x+y*y'
  [../]

  [./ffn]
    type = ParsedFunction
    expression = -4
  [../]
[]

[UserObjects]
  [./isuo]
    type = InsideUserObject
    variable = u
    diffusivity = diffusivity
    execute_on = 'initial timestep_end'
#    use_old_prop = true # Access a stateful material on an internal side
  [../]
[]

[Variables]
  [./u]
    family = LAGRANGE
    order = FIRST
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]

  [./ffn]
    type = BodyForce
    variable = u
    function = ffn
  [../]

  [./time]
    type = TimeDerivative
    variable = u
  [../]
[]

[BCs]
  [./all]
    type = FunctionDirichletBC
    variable = u
    boundary = '0 1 2 3'
    function = fn_exact
  [../]
[]

[Postprocessors]
  [./value]
    type = InsideValuePPS
    user_object = isuo
    execute_on = 'initial timestep_end'
  [../]
[]

[Executioner]
  type = Transient
  dt = 0.1
  num_steps = 5
[]

[Materials]
  [./stateful]
    type = StatefulMaterial
    block = 0
  [../]
[]

[Outputs]
  exodus = true
[]
