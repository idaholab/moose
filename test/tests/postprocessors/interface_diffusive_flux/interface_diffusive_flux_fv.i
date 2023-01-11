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
    primary_block = '1'
    paired_block = '0'
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
    type = MooseVariableFVReal
    block = 0
  []
  [v]
    type = MooseVariableFVReal
    block = 1
  []
[]

[FVKernels]
  [diff_u]
    type = FVDiffusion
    variable = u
    coeff = 1
  []
  [body_u]
    type = FVBodyForce
    variable = u
    function = 1
  []

  [diff_v]
    type = FVDiffusion
    variable = v
    coeff = 1
  []
  [body_v]
    type = FVBodyForce
    variable = v
    function = -1
  []
[]

[FVInterfaceKernels]
  [reaction]
    type = FVDiffusionInterface
    variable1 = u
    variable2 = v
    coeff1 = 1
    coeff2 = 2
    boundary = 'interface'
    subdomain1 = '0'
    subdomain2 = '1'
    coeff_interp_method = average
  []
[]

[FVBCs]
  [all]
    type = FVFunctionDirichletBC
    variable = u
    boundary = '0 1 2 3'
    function = fn_exact
  []
[]

[Postprocessors]
  [diffusive_flux]
    type = ${postprocessor_type}
    variable = v
    neighbor_variable = u
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
  file_base = '${raw ${postprocessor_type} _fv}'
  exodus = true
[]
