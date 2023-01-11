# Simple 3D test with diffusion, setup to make sure
# there is a sensible difference in the linear iteration
# counts with re-use versus without re-use

[Variables]
  [u]
  []
[]

[Mesh]
  [msh]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 4
    ny = 4
    nz = 4
  []
[]

[Kernels]
  [diffusion]
    type = FunctionDiffusion
    variable = u
    function = 'arg'
  []
  [time]
    type = TimeDerivative
    variable = u
  []
  [body_force]
    type = BodyForce
    variable = u
    function = body
  []
[]

[Functions]
  [body]
    type = ParsedFunction
    expression = 100*sin(t)
  []
  [arg]
    type = ParsedFunction
    expression = 'x*y*z*cos(t)+1'
  []
[]

[BCs]
  [fix_concentration]
    type = DirichletBC
    preset = true
    boundary = left
    variable = u
    value = 0.0
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient

  solve_type = 'newton'
  line_search = none

  petsc_options = ''
  petsc_options_iname = '-pc_type -ksp_type'
  petsc_options_value = 'lu gmres'
  l_tol = 1e-8
  l_max_its = 100

  reuse_preconditioner = false
  reuse_preconditioner_max_linear_its = 10

  nl_max_its = 10
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-10

  start_time = 0.0
  dt = 1.0
  dtmin = 1.0
  end_time = 10.0

  [./Adaptivity]
    interval = 5
    max_h_level = 1
    start_time = 11.0
    stop_time = 6.0
  [../]

[]

[Reporters/iteration_info]
  type = IterationInfo
[]

[Outputs]
  exodus = false
  [./csv]
    type = CSV
    file_base = base_case
  [../]
[]
