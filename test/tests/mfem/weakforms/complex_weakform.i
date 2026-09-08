# Include mfem/complex/complex.i
!include ../complex/complex.i

# Naming every kernel and boundary condition explicitly must reproduce the default weak form,
# which uses all of them.
[WeakForms]
  [Complex]
    type = MFEMComplexWeakForm
    kernels = 'diffusion_complex mass_complex'
    bcs = 'dbc'
  []
[]

[Postprocessors]
  [u_error]
    type = MFEMComplexL2Error
    variable = u
    function_real = u0_r
    function_imag = u0_i
  []
[]

[Outputs]
  [ComplexCSV]
    type = CSV
    file_base = OutputData/ComplexWeakForm
  []
[]
