[Mesh]
  # This test uses SolutionUserObject which doesn't work with DistributedMesh.
  type = GeneratedMesh
  parallel_type = replicated
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

[AuxVariables]
  [./u_aux]
  [../]
[]

[Functions]
  [./u_xdr_func]
    type = SolutionFunction
    solution = xdr_u
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[AuxKernels]
  [./aux_xdr_kernel]
    type = SolutionAux
    variable = u_aux
    solution = xdr_u_aux
    execute_on = initial
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
  [./xdr_u_aux]
    type = SolutionUserObject
    system = aux0
    mesh = aux_nonlinear_solution_xdr_0001_mesh.xdr
    es = aux_nonlinear_solution_xdr_0001.xdr
    execute_on = initial
  [../]
  [./xdr_u]
    type = SolutionUserObject
    system = nl0
    mesh = aux_nonlinear_solution_xdr_0001_mesh.xdr
    es = aux_nonlinear_solution_xdr_0001.xdr
    execute_on = initial
  [../]
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  nl_rel_tol = 1e-10
[]

[Outputs]
  exodus = true
[]

[ICs]
  [./u_func_ic]
    function = u_xdr_func
    variable = u
    type = FunctionIC
  [../]
[]
