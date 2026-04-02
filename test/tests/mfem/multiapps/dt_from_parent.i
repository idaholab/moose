[Problem]
  type = MFEMProblem
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
    expression = 'x + 2*(-sin(pi*x)*exp(-pi^2*t) + sin(2*pi*x)*exp(-4*pi^2*t)/2 - sin(3*pi*x)*exp(-9*pi^2*t)/3 + sin(4*pi*x)*exp(-16*pi^2*t)/4 - sin(5*pi*x)*exp(-25*pi^2*t)/5)/pi'
  []
[]

[Preconditioner]
  [boomeramg]
    type = MFEMHypreBoomerAMG
  []
[]

[Solvers]
  [main]
    type = MFEMHyprePCG
    preconditioner = boomeramg
    l_tol = 1e-8
    l_max_its = 100
  []
[]

[Executioner]
  type = MFEMTransient
  device = cpu
  num_steps = 10
  dt = 0.2
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
    file_base = dt_from_parent
  []
[]

[MultiApps]
  [sub_app]
    type = TransientMultiApp
    app_type = MooseTestApp
    input_files = 'dt_from_parent_sub.i'
  []
[]
