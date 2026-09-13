# Include mfem/complex/complex.i
!include ../complex/complex.i

# A real weak form cannot consume the complex boundary conditions of a complex problem.
[WeakForms]
  [Complex]
    type = MFEMWeakForm
    kernels = 'diffusion_complex mass_complex'
    bcs = 'dbc'
  []
[]
