[Mesh]
  [cmg]
    type = CartesianMeshGenerator
    dim = 2
    dx = '1 2'
    dy = '1.3'
    ix = '5 10'
    iy = '3'
    subdomain_id = '0 1'
  []
  [mid]
    type = SideSetsBetweenSubdomainsGenerator
    input = 'cmg'
    primary_block = '0'
    paired_block = '1'
    new_boundary = 'mid'
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
  [diffu]
    type = FVDiffusion
    variable = u
    coeff = 1
  []
  [diffv]
    type = FVDiffusion
    variable = v
    coeff = 2
    block = 1
  []
  [source]
    type = FVBodyForce
    variable = v
    value = 1
  []
[]

[FVBCs]
  [left]
    type = FVDirichletBC
    variable = u
    boundary = left
    value = 4
  []
  [mid]
    type = FVADFunctorDirichletBC
    variable = u
    functor = v
    functor_only_defined_on_other_side = true
    ghost_layers = 3
    boundary = mid
  []
  [right]
    type = FVDirichletBC
    variable = v
    boundary = right
    value = 0.5
  []
[]

[Executioner]
  type = Steady
  solve_type = 'Newton'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  nl_abs_tol = 1e-12
[]

[VectorPostprocessors]
  [u_sample]
    type = LineValueSampler
    variable = 'u'
    start_point = '0.01 0.3 0'
    end_point = '0.99 0.3 0'
    num_points = 4
    sort_by = x
  []
  [v_sample]
    type = LineValueSampler
    variable = 'v'
    start_point = '1.01 0.3 0'
    end_point = '1.99 0.3 0'
    num_points = 4
    sort_by = x
  []
[]

[Outputs]
  exodus = true
  csv = true
[]
