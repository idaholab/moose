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
  [source]
    type = MFEMDomainLFKernel
    variable = u
    coefficient = forcing
  []
[]

[BCs]
  [left]
    type = MFEMScalarDirichletBC
    variable = u
    boundary = left
    coefficient = exact_solution
  []
  [right]
    type = MFEMScalarDirichletBC
    variable = u
    boundary = right
    coefficient = exact_solution
  []
  [top]
    type = MFEMScalarDirichletBC
    variable = u
    boundary = top
    coefficient = exact_solution
  []
  [bottom]
    type = MFEMScalarDirichletBC
    variable = u
    boundary = bottom
    coefficient = exact_solution
  []
[]

[Functions]
  [exact_solution]
    type = ParsedFunction
    # Chosen exact solution for the steady Poisson problem, u(x,y) = x^2 + y^2.
    expression = 'x*x + y*y'
  []
  [forcing]
    type = ParsedFunction
    # For -Delta u = f, this exact solution gives f = -(2 + 2) = -4.
    expression = '-4'
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
  type = MFEMSteady
[]

[Postprocessors]
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
    file_base = full_solve_sub
  []
[]
