d1 = 1
d2 = 10

[Mesh]
  [gen_mesh]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
    elem_type = TRI3
  []
  [subdomain]
    type = ParsedSubdomainMeshGenerator
    input = gen_mesh
    combinatorial_geometry = 'y > 0.5'
    block_id = 1
  []
[]

[Variables]
  [v]
    type = MooseVariableFVReal
  []
[]

[FVKernels]
  [diff]
    type = FVDiffusion
    variable = v
    coeff = 'diff_coeff'
    coeff_interp_method = average
  []
  [body_v]
    type = FVBodyForce
    variable = v
    function = 'forcing'
  []
[]

[FVBCs]
  [exact]
    type = FVFunctionDirichletBC
    boundary = 'left right top bottom'
    function = 'exact'
    variable = v
  []
[]

[Functions]
  [exact]
    type = ParsedFunction
    expression = 'if (y < 0.5, 1 + x + 3*y*y*y, (11*d2-3*d1)/ (8*d2) + x + 3*d1/d2*y*y*y)'
    symbol_names = 'd1 d2'
    symbol_values = '${d1} ${d2}'
  []
  [forcing]
    type = ParsedFunction
    expression = '-d1*18*y'
    symbol_names = 'd1'
    symbol_values = '${d1}'
  []
[]

[Materials]
  [diff_coeff]
    type = ADPiecewiseByBlockFunctorMaterial
    prop_name = 'diff_coeff'
    subdomain_to_prop_value = '0 ${d1}
                               1 ${d2}'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  csv = true
  exodus = true
[]

[Postprocessors]
  [error]
    type = ElementL2Error
    variable = v
    function = exact
    outputs = 'console csv'
  []
  [h]
    type = AverageElementSize
    outputs = 'console csv'
  []
[]
