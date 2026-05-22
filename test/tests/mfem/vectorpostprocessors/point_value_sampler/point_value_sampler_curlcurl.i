# Definite Maxwell problem solved with Nedelec elements of the first kind
# based on MFEM Example 3. Sampled with MFEMPointValueSampler.

!include ../../kernels/curlcurl.i

[VectorPostprocessors]
  [point_sample]
    type = MFEMPointValueSampler
    variable = 'e_field'
    points = '1 1 -0.5  1 1 0.5'
  []
[]

[Outputs]
  csv = true
[]
