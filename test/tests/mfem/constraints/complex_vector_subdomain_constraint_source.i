# Complex (time-harmonic) analogue of vector_subdomain_constraint_source.i. A
# complex first-kind Nedelec field E on the 2D hinomaru mesh is governed by
# curl(curl E) + E = 0 with its tangential trace pinned to zero on the outer
# boundary. An MFEMComplexVectorEssentialConstraint strongly constrains the real
# part of E to (2, 3) and the imaginary part to (4, 5) inside the "wire"
# subdomain. With a purely real operator the two parts decouple, so each is an
# extension of its subdomain and boundary values.

[Mesh]
  type = MFEMMesh
  file = ../mesh/hinomaru.e
[]

[Problem]
  type = MFEMProblem
  numeric_type = complex
[]

[FESpaces]
  [HCurlFESpace]
    type = MFEMVectorFESpace
    fec_type = ND
    fec_order = FIRST
  []
[]

[Variables]
  [E]
    type = MFEMComplexVariable
    fespace = HCurlFESpace
  []
[]

[BCs]
  [tangential_E]
    type = MFEMComplexVectorTangentialDirichletBC
    variable = E
    boundary = outer
    vector_coefficient_real = '0.0 0.0'
    vector_coefficient_imag = '0.0 0.0'
  []
[]

[Constraints]
  [circle_interior]
    type = MFEMComplexVectorEssentialConstraint
    variable = E
    block = wire
    vector_coefficient_real = '2.0 3.0'
    vector_coefficient_imag = '4.0 5.0'
  []
[]

[Kernels]
  [curlcurl]
    type = MFEMComplexKernel
    variable = E
    [RealComponent]
      type = MFEMCurlCurlKernel
    []
  []
  [mass]
    type = MFEMComplexKernel
    variable = E
    [RealComponent]
      type = MFEMVectorFEMassKernel
    []
  []
[]

[Solvers]
  [main]
    type = MFEMMUMPS
  []
[]

[Executioner]
  type = MFEMSteady
  device = cpu
  assembly_level = legacy
[]

[VectorPostprocessors]
  [point_sample]
    type = MFEMComplexVariablePointValueSampler
    variable = 'E'
    points = '0.0 0.0 0.0  1.5 0.0 0.0  2.5 0.0 0.0'
  []
[]

[Outputs]
  csv = true
[]
