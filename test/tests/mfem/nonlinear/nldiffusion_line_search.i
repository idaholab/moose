# Nonlinear diffusion solved from the constant initial guess supplied as a command line argument
# by the tests using this input, which lies well below the boundary values, so that the first full
# Newton updates overshoot the solution by orders of magnitude. The iteration limit set below
# suffices only if those updates are shortened by a line search; the undamped solve needs twelve
# iterations to recover from the overshoot.
!include nldiffusion_common.i

[Solvers]
  [boomeramg]
    type = MFEMHypreBoomerAMG
  []
  [lin]
    type = MFEMHyprePCG
    preconditioner = boomeramg
    print_level = 1
    l_tol = 1e-12
    l_max_its = 1000
  []
  [native_mfem_nl]
    type = MFEMNewtonNonlinearSolver
    max_its = 8
    abs_tol = 1.0e-10
    rel_tol = 1.0e-9
    print_level = 1
    line_search = backtracking
  []
[]
