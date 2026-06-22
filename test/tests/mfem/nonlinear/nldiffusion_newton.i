!include nldiffusion_common.i

[Preconditioner]
  [boomeramg]
    type = MFEMHypreBoomerAMG
  []
[]

[Solvers]
  [lin]
    type = MFEMHypreGMRES
    preconditioner = boomeramg
    print_level = 1
    l_tol = 1e-12
    l_max_its = 1000
  []
  [native_mfem_nl]
    type = MFEMNewtonNonlinearSolver
    max_its = 100
    abs_tol = 1.0e-10
    rel_tol = 1.0e-9
    print_level = 1
  []
[]
