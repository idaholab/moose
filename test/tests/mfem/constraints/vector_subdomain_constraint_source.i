# Vector analogue of subdomain_constraint_source.i. A first-kind Nedelec field E
# on the 2D hinomaru mesh is governed by the definite operator
# curl(curl E) + E = 0 with its tangential trace pinned to zero on the outer
# boundary. An MFEMVectorEssentialConstraint strongly constrains E to the
# constant vector (2, 3) inside the "wire" subdomain; the surrounding material
# carries the resulting decaying field.
#
# Lowest-order Nedelec elements reproduce a constant vector field exactly, so the
# sampled field is exactly (2, 3) at the point inside the constrained subdomain.

[Mesh]
  type = MFEMMesh
  file = ../mesh/hinomaru.e
[]

[Problem]
  type = MFEMProblem
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
    type = MFEMVariable
    fespace = HCurlFESpace
  []
[]

[BCs]
  [tangential_E]
    type = MFEMVectorTangentialDirichletBC
    variable = E
    boundary = outer
    vector_coefficient = '0.0 0.0'
  []
[]

[Constraints]
  [circle_interior]
    type = MFEMVectorEssentialConstraint
    variable = E
    block = wire
    vector_coefficient = '2.0 3.0'
  []
[]

[Kernels]
  [curlcurl]
    type = MFEMCurlCurlKernel
    variable = E
  []
  [mass]
    type = MFEMVectorFEMassKernel
    variable = E
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
    type = MFEMVariablePointValueSampler
    variable = 'E'
    points = '0.0 0.0 0.0  1.5 0.0 0.0  2.5 0.0 0.0'
  []
[]

[Outputs]
  csv = true
[]
