# MFEM diffusion problem sampled with MFEMLineVariableValueSampler.

!include ../../kernels/diffusion.i

[VectorPostprocessors]
  [line_sample]
    type = MFEMLineVariableValueSampler
    variable = 'concentration'
    start_point = '2.125 0 -2.375'
    end_point = '2.125 0 2.625'
    num_points = 11
  []
[]

[Outputs]
  csv = true
[]
