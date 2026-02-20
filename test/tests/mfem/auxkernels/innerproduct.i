!include ../kernels/curlcurl.i

[FESpaces]
  [L2FESpace]
    type = MFEMScalarFESpace
    fec_type = L2
    fec_order = CONSTANT
  []
[]

[AuxVariables]
  [joule_heating]
    type = MFEMVariable
    fespace = L2FESpace
  []
[]

[AuxKernels]
  [joule_Q_aux]
    type = MFEMInnerProductAux
    variable = joule_heating
    first_source_vec = e_field
    second_source_vec = e_field
    execute_on = TIMESTEP_END
  []
[]

[VectorPostprocessors]
  active=line_sample
  [line_sample]
    type=MFEMLineValueSampler
    variable=joule_heating
    start_point="-0.99 -0.99 0.99"
    end_point="0.99 0.99 -0.99"
    num_points=114
  []
[]
