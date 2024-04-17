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
    type = DerivativeParsedMaterial
    property_name = forcing_material
    extra_symbols = x
    coupled_variables = alphapi
    expression = 'alphapi*alphapi*sin(alphapi*x)'
  []
[]

[Kernels]
  [alphapi]
    type = Diffusion
    variable = alphapi
  []
  [diff]
    type = Diffusion
    variable = u
  []
  [forcing]
    type = MatBodyForce
    variable = u
    material_property = forcing_material
    coupled_variables = alphapi
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = right
    value = 0
  []
  [right]
    type = DirichletBC
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
