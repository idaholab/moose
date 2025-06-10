[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 4
    ny = 1
    elem_type = QUAD9
    xmax = 4
  []
  [different_sub]
    type = SubdomainBoundingBoxGenerator
    block_id = 1
    bottom_left = '2 0 0'
    input = gen
    top_right = '4 1 0'
  []
  [left_sideset_for_right_sub]
    type = SideSetsBetweenSubdomainsGenerator
    input = different_sub
    new_boundary = left_1
    paired_block = 0
    primary_block = 1
  []
  [break]
    type = BreakBoundaryOnSubdomainGenerator
    input = left_sideset_for_right_sub
  []
  [right_sideset_for_left_sub]
    type = SideSetsBetweenSubdomainsGenerator
    input = break
    new_boundary = right_0
    paired_block = 1
    primary_block = 0
  []
[]

[Problem]
  nl_sys_names = 'nl0 nl1'
  kernel_coverage_check = false
[]

[Variables]
  [u]
    order = FIRST
    family = MONOMIAL
    block = 0
    solver_sys = 'nl0'
  []
  [side_u]
    order = FIRST
    family = SIDE_HIERARCHIC
    block = 0
    solver_sys = 'nl0'
  []
  [v]
    order = FIRST
    family = MONOMIAL
    block = 1
    solver_sys = 'nl1'
  []
[]

[Kernels]
  [diff_v]
    type = Diffusion
    variable = v
  []
[]

[HDGKernels]
  [diff]
    type = DiffusionIPHDGKernel
    variable = u
    face_variable = side_u
    diffusivity = 1
    alpha = 6
  []
[]

[DGKernels]
  [diff]
    type = DGDiffusion
    epsilon = -1
    sigma = 6
    variable = v
  []
[]

[BCs]
  [left_u]
    type = DiffusionIPHDGDirichletBC
    functor = 0
    boundary = 'left'
    alpha = 6
    variable = u
    face_variable = side_u
    diffusivity = 1
  []
  [zero_flux_u]
    type = DiffusionIPHDGPrescribedFluxBC
    boundary = 'top_to_0 bottom_to_0'
    prescribed_normal_flux = 0
    variable = u
    face_variable = side_u
    alpha = 6
    diffusivity = 1
  []
  [right_u]
    type = DiffusionIPHDGDirichletBC
    functor = 1
    boundary = 'right_0'
    alpha = 6
    variable = u
    face_variable = side_u
    diffusivity = 1
  []
  [left_v]
    type = DGFunctionDiffusionDirichletBC
    boundary = left_1
    epsilon = -1
    function = 0
    sigma = 6
    variable = v
  []
  [right_v]
    type = DGFunctionDiffusionDirichletBC
    boundary = right
    epsilon = -1
    function = 1
    sigma = 6
    variable = v
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu       mumps'
[]

[Outputs]
  csv = true
[]

[Postprocessors]
  [avg_u]
    type = ElementAverageValue
    variable = u
    block = 0
  []
  [avg_side_u]
    type = ElementAverageValue
    variable = side_u
    block = 0
  []
  [avg_v]
    type = ElementAverageValue
    variable = v
    block = 1
  []
[]
