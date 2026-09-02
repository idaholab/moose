# MFEM diffusion problem sampled with MFEMVariableLineValueSampler.

!include ../../kernels/diffusion.i

[VectorPostprocessors]
  [line_sample]
    type = MFEMVariableLineValueSampler
    variable = 'concentration'
    start_point = '2.125 0 -2.375'
    end_point = '2.125 0 2.625'
    num_points = 11
  []
[]

[Outputs]
  csv = true
[]
