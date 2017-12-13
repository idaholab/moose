[Mesh]
  file = circle-quads.e
[]

[Functions]
  [./all_bc_fn]
    type = ParsedFunction
    value = x*x+y*y
  [../]

  [./f_fn]
    type = ParsedFunction
    value = -4
  [../]

  [./analytical_normal_x]
    type = ParsedFunction
    value = x
  [../]
  [./analytical_normal_y]
    type = ParsedFunction
    value = y
  [../]
[]

[NodalNormals]
  [./nodal_normals]
    boundary = 1
  [../]
[]

[AuxVariables]
  [./nodal_normal_x]
  [../]
  [./nodal_normal_y]
  [../]
  [./nodal_normal_z]
  [../]
[]

[AuxKernels]
  [./nnx_left]
    type = NodalNormalAux
    variable = nodal_normal_x
    component = X
    nodal_normals = nodal_normals
    execute_on = timestep_end
  [../]
  [./nny_left]
    type = NodalNormalAux
    variable = nodal_normal_y
    component = Y
    nodal_normals = nodal_normals
    execute_on = timestep_end
  [../]
  [./nnz_left]
    type = NodalNormalAux
    variable = nodal_normal_z
    component = Z
    nodal_normals = nodal_normals
    execute_on = timestep_end
  [../]
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
  [./ffn]
    type = BodyForce
    variable = u
    function = f_fn
  [../]
[]

[BCs]
  [./all]
    type = FunctionDirichletBC
    variable = u
    boundary = '1'
    function = 'all_bc_fn'
  [../]
[]

[Postprocessors]
  [./nx_pps]
    type = NodalL2Error
    variable = nodal_normal_x
    boundary = '1'
    function = analytical_normal_x
  [../]
  [./ny_pps]
    type = NodalL2Error
    variable = nodal_normal_y
    boundary = '1'
    function = analytical_normal_y
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
  nl_rel_tol = 1e-13
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
[]
