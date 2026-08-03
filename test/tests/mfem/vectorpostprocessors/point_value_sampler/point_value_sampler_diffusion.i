# MFEM diffusion problem sampled with MFEMPointVariableValueSampler.

!include ../../kernels/diffusion.i

[VectorPostprocessors]
  [point_sample]
    type = MFEMPointVariableValueSampler
    variable = 'concentration'
    points = '2.125 0 -1.375  2.125 0 1.125'
  []
[]

[Outputs]
  csv = true
[]
