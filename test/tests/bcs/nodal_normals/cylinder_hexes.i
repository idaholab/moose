[Mesh]
  file = cylinder-hexes.e
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
[]

[NodalNormals]
  [./nodal_normals]
    boundary = '1'
    corner_boundary = 100
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

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
  nl_rel_tol = 1e-13
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
[]
