# Definite Maxwell problem solved with Nedelec elements of the first kind
# based on MFEM Example 3. Sampled with MFEMPointVariableValueSampler.

!include ../../kernels/curlcurl.i

[VectorPostprocessors]
  [point_sample]
    type = MFEMPointVariableValueSampler
    variable = 'e_field'
    points = '0.99 0.99 -0.49  0.99 0.99 0.49'
  []
[]

[Outputs]
  csv = true
[]
