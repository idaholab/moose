# Definite Maxwell problem solved with Nedelec elements of the first kind
# based on MFEM Example 3. Sampled with MFEMLineVariableValueSampler.

!include ../../kernels/curlcurl.i

[VectorPostprocessors]
  [line_sample]
    type = MFEMLineVariableValueSampler
    variable = 'e_field'
    start_point = '0.99 0.99 -0.99'
    end_point = '0.99 0.99 0.99'
    num_points = 12
  []
[]

[Outputs]
  csv = true
[]
