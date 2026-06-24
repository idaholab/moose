# Definite Maxwell problem solved with Nedelec elements of the first kind
# based on MFEM Example 3. Sampled with MFEMLineValueSampler.

!include ../../kernels/curlcurl.i

[VectorPostprocessors]
  [line_sample]
    type = MFEMLineValueSampler
    variable = 'e_field'
    start_point = '1 1 -1'
    end_point = '1 1 1'
    num_points = 11
  []
[]

[Outputs]
  csv = true
[]
