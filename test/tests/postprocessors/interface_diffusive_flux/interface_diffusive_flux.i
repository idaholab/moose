postprocessor_type = InterfaceDiffusiveFluxAverage

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 6
    xmax = 3
    ny = 9
    ymax = 3
    elem_type = QUAD4
  []
  [subdomain_id]
    input = gen
    type = SubdomainBoundingBoxGenerator
    bottom_left = '0 0 0'
    top_right = '2 1 0'
    block_id = 1
  []
  [interface]
    input = subdomain_id
    type = SideSetsBetweenSubdomainsGenerator
    primary_block = '0'
    paired_block = '1'
    new_boundary = 'interface'
  []
[]

[Functions]
  [fn_exact]
    type = ParsedFunction
    expression = 'x*x+y*y'
  []
[]

[Variables]
  [u]
    block = 0
  []
  [v]
    block = 1
  []
[]


[Kernels]
  [diff_u]
    type = Diffusion
    variable = u
  []
  [body_u]
    type = BodyForce
    variable = u
    function = 1
  []

  [diff_v]
    type = Diffusion
    variable = v
  []
  [body_v]
    type = BodyForce
    variable = v
    function = -1
  []
[]

# Not a diffusion interface but can test the postprocessor anyway
[InterfaceKernels]
  [reaction]
    type = InterfaceReaction
    kb = 1
    kf = 2
    variable = u
    neighbor_var = v
    boundary = 'interface'
  []
[]

[BCs]
  [all]
    type = FunctionDirichletBC
    variable = u
    boundary = '0 1 2 3'
    function = fn_exact
  []
[]

[Postprocessors]
  [diffusive_flux]
    type = ${postprocessor_type}
    variable = u
    neighbor_variable = v
    diffusivity = 1
    execute_on = TIMESTEP_END
    boundary = 'interface'
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Outputs]
  file_base = ${raw ${postprocessor_type} _fe}
  exodus = true
[]
