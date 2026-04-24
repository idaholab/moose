[Problem]
  type = MFEMProblem
  verbose_multiapps = true
[]

[Mesh]
  type = MFEMFileMesh
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
    # w in a sine series with n-th term: (-1)^n*(2/n/pi)*sin(n*pi*x)*exp(-n^2*pi^2*t).
    # This expression keeps the first five terms of that series.
    expression = 'x + 2*(-sin(pi*x)*exp(-pi^2*t) + sin(2*pi*x)*exp(-4*pi^2*t)/2 - sin(3*pi*x)*exp(-9*pi^2*t)/3 + sin(4*pi*x)*exp(-16*pi^2*t)/4 - sin(5*pi*x)*exp(-25*pi^2*t)/5)/pi'
  []
[]


[Solvers]
  [boomeramg]
    type = MFEMHypreBoomerAMG
  []
  [main]
    type = MFEMHyprePCG
    preconditioner = boomeramg
    l_tol = 1e-12
    l_max_its = 100
  []
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
