S = 10
D = 10
L = 5

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 50
    ny = 10
    xmax = ${L}
    ymin = -1
    ymax = 1
  []

[]

[Problem]
  solve = false
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diffusion_u]
    type = ADMatDiffusion
    variable = u
    diffusivity = D_u
  []
  [source_u]
    type = ADBodyForce
    variable = u
    value = 1.0
  []
[]

[Functions]
  [du]
    type = ParsedFunction
    expression = 'D * x^2 + 1'
    symbol_names = D
    symbol_values = ${D}
  []
[]

[Materials]
  [diffusivity_u]
    type = ADGenericFunctionMaterial
    prop_names = D_u
    prop_values = du
  []
[]

[BCs]
  [left_u]
    type = DirichletBC
    variable = u
    boundary = 'left top bottom'
    value = 0
  []
  [right_u]
    type = FunctionDirichletBC
    variable = u
    boundary = right
    function = '${S} * t^2 * -(y^2-1)'
    preset = false
  []
[]

[Executioner]
  type = Transient
  dt = 0.25
  num_steps = 4
  solve_type = NEWTON
[]

[VariableMappings]
  [rb_mapping]
    type = DEIMRBMapping
    filename = 'parallel_storage_main_mapping_rb_mapping.rd'
  []
[]

[UserObjects]
  [im]
    type = InverseRB
    mapping = rb_mapping
    execute_on = TIMESTEP_END
    max_iter = 2
  []
[]

[Outputs]
  exodus = true
[]
