[Problem]
  type = MFEMProblem
  verbose_multiapps = true
[]

[Mesh]
  type = MFEMMesh
  file = ../mesh/square.e
[]

[FESpaces]
  [H1FESpace]
    type = MFEMScalarFESpace
    fec_type = H1
    fec_order = FIRST
  []
[]

[Variables]
  [u]
    type = MFEMVariable
    fespace = H1FESpace
  []
[]

[Kernels]
  [diff]
    type = MFEMDiffusionKernel
    variable = u
  []
  [td]
    type = MFEMTimeDerivativeMassKernel
    variable = u
  []
[]

[BCs]
  [left]
    type = MFEMScalarDirichletBC
    variable = u
    boundary = left
    coefficient = 0
  []
  [right]
    type = MFEMScalarDirichletBC
    variable = u
    boundary = right
    coefficient = 1
  []
[]

[Functions]
  [exact_solution]
    type = ParsedFunction
    # Exact solution for u_t - u_xx = 0 on x in [0,1] with u(0,t)=0 and u(1,t)=1.
    # Write u(x,t) = x + w(x,t), so w has homogeneous Dirichlet data, then expand
    # w in a sine series. This expression keeps the first five terms of that series.
    expression = 'x + 2*(-sin(pi*x)*exp(-pi^2*t) + sin(2*pi*x)*exp(-4*pi^2*t)/2 - sin(3*pi*x)*exp(-9*pi^2*t)/3 + sin(4*pi*x)*exp(-16*pi^2*t)/4 - sin(5*pi*x)*exp(-25*pi^2*t)/5)/pi'
  []
[]


[Solvers]
  [boomeramg]
    type = MFEMHypreBoomerAMG
      print_level = 0
  []
  [main]
    type = MFEMHyprePCG
    preconditioner = boomeramg
    l_tol = 1e-8
    l_max_its = 100
    print_level = 0
  []
[]

[Executioner]
  type = MFEMTransient
  device = cpu
  num_steps = 2
  dt = 0.1
[]

[Postprocessors]
  [dt]
    type = TimestepSize
    execute_on = TIMESTEP_END
  []
  [error]
    type = MFEML2Error
    variable = u
    function = exact_solution
    execute_on = TIMESTEP_END
  []
[]

[Outputs]
  [CSV]
    type = CSV
    execute_on = TIMESTEP_END
    file_base = full_solve_parent
  []
[]

[MultiApps]
  [full_solve]
    type = FullSolveMultiApp
    app_type = MooseTestApp
    execute_on = timestep_begin
    input_files = full_solve_sub.i
    keep_full_output_history = true
  []
[]
