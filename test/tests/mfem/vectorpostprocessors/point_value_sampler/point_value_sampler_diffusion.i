# MFEM diffusion problem sampled with MFEMVariablePointValueSampler.

!include ../../kernels/diffusion.i

[VectorPostprocessors]
  [point_sample]
    type = MFEMVariablePointValueSampler
    variable = 'concentration'
    points = '2.125 0 -1.375  2.125 0 1.125'
  []
[]

[Outputs]
  csv = true
[]
