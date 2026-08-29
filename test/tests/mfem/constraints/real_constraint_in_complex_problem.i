# Error case: a real constraint used in a complex (time-harmonic) problem. The
# complex equation system keeps its constraints in a separate map, so registering
# a real constraint would leave it silently unapplied.

[Mesh]
  type = MFEMMesh
  file = ../mesh/hinomaru.e
[]

[Problem]
  type = MFEMProblem
  numeric_type = complex
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
    type = MFEMComplexVariable
    fespace = H1FESpace
  []
[]

[Constraints]
  [circle_interior]
    type = MFEMScalarEssentialConstraint
    variable = temperature
    block = wire
    coefficient = 2.0
  []
[]

[Kernels]
  [diff]
    type = MFEMComplexKernel
    variable = temperature
    [RealComponent]
      type = MFEMDiffusionKernel
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
