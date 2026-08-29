# Error case: 'block' names a numeric subdomain attribute that the mesh does not
# have. Without the check this would silently constrain nothing.

[Mesh]
  type = MFEMMesh
  file = ../mesh/hinomaru.e
[]

[Problem]
  type = MFEMProblem
[]

[FESpaces]
  [H1FESpace]
    type = MFEMScalarFESpace
    fec_type = H1
    fec_order = FIRST
  []
[]

[Variables]
  [temperature]
    type = MFEMVariable
    fespace = H1FESpace
  []
[]

[Constraints]
  [circle_interior]
    type = MFEMScalarEssentialConstraint
    variable = temperature
    block = '99'
    coefficient = 2.0
  []
[]

[Kernels]
  [diff]
    type = MFEMDiffusionKernel
    variable = temperature
  []
[]

[Executioner]
  type = MFEMSteady
  device = cpu
[]
