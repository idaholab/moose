[Mesh]
  # This test uses SolutionUserObject which doesn't work with ParallelMesh.
  type = GeneratedMesh
  parallel_type = REPLICATED
  dim = 2
  nx = 2
  ny = 2
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Functions]
  [./u_xda_func]
    type = SolutionFunction
    solution = xda_u
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 1
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = 2
    value = 2
  [../]
[]

[UserObjects]
  [./xda_u]
    type = SolutionUserObject
    system = nl0
    mesh = aux_nonlinear_solution_out_0001_mesh.xda
    es = aux_nonlinear_solution_out_0001.xda
    system_variables = u
    execute_on = initial
  [../]
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  nl_rel_tol = 1e-10
[]

[Postprocessors]
  [./unorm]
    type = ElementL2Norm
    variable = u
  [../]
  [./uerror]
    type = ElementL2Error
    variable = u
    function = u_xda_func
  [../]
[]

[Outputs]
  csv = true
[]
