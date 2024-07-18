[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 5
  ny = 5
  xmax = 5
  ymax = 5
[]

[Variables/u]
  order = FIRST
  family = LAGRANGE
[]

[BCs/zero]
  type = DirichletBC
  variable = u
  value = 0
  boundary = 'top right bottom left'
[]

[Kernels/diffusion]
  type = Diffusion
  variable = u
[]

[Postprocessors/postprocessor]
  type = FunctionValuePostprocessor
  function = 3
  execute_on = initial
[]

[RayKernels]
  [constant_source]
    type = LineSourceRayKernel
    variable = u
    value = 5
    rays = constant_source
  []
  [pp_source]
    type = LineSourceRayKernel
    variable = u
    postprocessor = postprocessor
    rays = pp_source
  []
  [function_source]
    type = LineSourceRayKernel
    variable = u
    function = 'x + 2 * y'
    rays = function_source
  []
  [mixed_source]
    type = LineSourceRayKernel
    variable = u
    value = 5
    postprocessor = postprocessor
    function = 'x + 2 * y'
    rays = mixed_source
  []
  [data_source]
    type = LineSourceRayKernel
    variable = u
    ray_data_factor_names = data
    rays = data_source
  []
  [aux_data_source]
    type = LineSourceRayKernel
    variable = u
    ray_aux_data_factor_names = aux_data
    rays = aux_data_source
  []
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
