[Mesh]
  [square]
    type = GeneratedMeshGenerator
    nx = 2
    ny = 2
    dim = 2
  []
  uniform_refine = 4
[]

[Variables]
  [u]
  []
  [alphapi]
    initial_condition = ${fparse 16 * 3.14159265359}
  []
[]

[Materials]
  [forcing_material]
    type = ADDerivativeParsedMaterial
    property_name = forcing_material
    extra_symbols = x
    coupled_variables = alphapi
    expression = 'alphapi*alphapi*sin(alphapi*x)'
  []
[]

[Kernels]
  [alphapi]
    type = ADDiffusion
    variable = alphapi
  []
  [diff]
    type = ADDiffusion
    variable = u
  []
  [forcing]
    type = ADMatBodyForce
    variable = u
    material_property = forcing_material
  []
[]

[BCs]
  [left]
    type = ADDirichletBC
    variable = u
    boundary = right
    value = 0
  []
  [right]
    type = ADDirichletBC
    variable = u
    boundary = left
    value = 0
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  nl_rel_tol = 1e-12
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
  hide = alphapi
[]
