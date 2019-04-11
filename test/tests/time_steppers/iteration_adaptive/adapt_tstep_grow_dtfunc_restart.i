[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 50
  ny = 2
  xmax = 5
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
  [./dt]
    type = TimeDerivative
    variable = u
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 10
  [../]
  [./right]
    type = NeumannBC
    variable = u
    boundary = right
    value = -1
  [../]
[]

[Executioner]
  type = Transient
  solve_type = PJFNK
  verbose = true
  petsc_options = -snes_ksp_ew
  petsc_options_iname = -ksp_gmres_restart
  petsc_options_value = 101
  line_search = none
  nl_rel_tol = 1e-8
  end_time = 20.0
  [./TimeStepper]
    type = IterationAdaptiveDT
    dt = 1.0
    optimal_iterations = 10
    time_t = '0.0 5.0'
    time_dt = '1.0 5.0'
  [../]
[]

[Postprocessors]
  [./_dt]
    type = TimestepSize
  [../]
[]

[Outputs]
  exodus = true
  checkpoint = true
[]

[Problem]
  restart_file_base = adapt_tstep_grow_dtfunc_out_cp/0003
[]
