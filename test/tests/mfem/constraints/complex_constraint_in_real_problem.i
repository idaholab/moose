# Error case: a complex constraint used in a real (non time-harmonic) problem,
# which has no complex equation system to register it with.

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
    type = MFEMComplexScalarEssentialConstraint
    variable = temperature
    block = wire
    coefficient_real = 2.0
    coefficient_imag = 3.0
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
