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
[]

[Materials]
  [forcing_material]
    type = ParsedMaterial
    property_name = forcing_material
    extra_symbols = x
    expression = 'alpha*alpha*pi*pi*sin(alpha*pi*x)'
    constant_names = 'alpha pi'
    constant_expressions = '16 3.14159265359'
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
  [forcing]
    type = MatBodyForce
    variable = u
    material_property = forcing_material
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
  nl_rel_tol = 1e-12
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
[]
