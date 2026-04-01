!include nldiffusion_common.i

[Solvers]
  [petsc_nl]
    type = MFEMPetscNonlinearSolver
    max_its = 100
    abs_tol = 1.0e-10
    rel_tol = 1.0e-9
    print_level = 1
    petsc_options_iname = '-pc_type'
    petsc_options_value = 'hypre'
  []
[]
