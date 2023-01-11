[Mesh]
  [gen]
  type = GeneratedMeshGenerator
    dim = 2
    xmin = -1
    ymin = -1
    xmax = 1
    ymax = 1
    nx = 2
    ny = 2
    elem_type = QUAD4
  []
  [./subdomain_id]
    input = gen
    type = ElementSubdomainIDGenerator
    subdomain_ids = '0 1
                     1 1'
  [../]
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
[]

[BCs]
  [./all]
    type = FunctionDirichletBC
    variable = u
    boundary = '0 1 2 3'
    function = fn_exact
  [../]
[]

[Materials]
  [./stateful1]
    type = StatefulMaterial
    block = 0
    initial_diffusivity = 1
  [../]
  [./stateful2]
    type = StatefulMaterial
    block = 1
    initial_diffusivity = 2
  [../]
[]

[Postprocessors]
  [./value]
    type = InsideValuePPS
    user_object = isuo
  [../]
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
