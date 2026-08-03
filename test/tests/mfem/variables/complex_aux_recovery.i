!include mfem_variables_from_moose.i

[FESpaces/ComplexH1FESpace]
  type = MFEMScalarFESpace
  fec_type = H1
  fec_order = FIRST
[]

[AuxVariables/complex_state]
  type = MFEMComplexVariable
  fespace = ComplexH1FESpace
[]

[AuxKernels/project_complex_state]
  type = MFEMComplexScalarProjectionAux
  variable = complex_state
  coefficient_real = 1.0
  coefficient_imag = 2.0
  execute_on = INITIAL
[]

[VectorPostprocessors/complex_state_sample]
  type = MFEMComplexPointVariableValueSampler
  variable = complex_state
  points = '2.125 0 -1.375'
  execute_on = TIMESTEP_END
[]
