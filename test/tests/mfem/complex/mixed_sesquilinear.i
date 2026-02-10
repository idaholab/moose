[Mesh]
  type = MFEMMesh
  file = ../mesh/inline-quad.mesh
  dim = 2
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
  [u]
    type = MFEMComplexVariable
    fespace = H1FESpace
  []
[]

[Functions]
  [u0_r]
    type = ParsedFunction
    expression = exp(y*${kappa_i})*cos(y*${kappa_r})
  []
  [u0_i]
    type = ParsedFunction
    expression = -exp(y*${kappa_i})*sin(y*${kappa_r})
  []
  [stiffnessCoef]
    type = ParsedFunction
    expression = 1.0/${mu}
  []
  [massCoef]
    type = ParsedFunction
    expression = -${omega}*${omega}*${epsilon}
  []
  [lossCoef]
    type = ParsedFunction
    expression = ${omega}*${sigma}
  []
[]

[BCs]
  [dbc]
    type = MFEMComplexScalarDirichletBC
    variable = u
    coefficient_real = u0_r
    coefficient_imag = u0_i
  []
[]

[Kernels]
  [diffusion_complex]
    type = MFEMComplexKernel
    variable = u
    [RealComponent]
      type = MFEMDiffusionKernel
      coefficient = stiffnessCoef
    []
    [ImagComponent]
      type = MFEMDiffusionKernel
      coefficient = 0.0
    []
  []
  [mass_complex]
    type = MFEMComplexKernel
    variable = u
    [RealComponent]
      type = MFEMMassKernel
      coefficient = massCoef
    []
    [ImagComponent]
      type = MFEMMassKernel
      coefficient = lossCoef
    []
  []
[]

[Solver]
  type = MFEMSuperLU
[]

[Executioner]
  type = MFEMSteady
  assembly_level = legacy
[]

[Outputs]
  [ParaViewDataCollection]
    type = MFEMParaViewDataCollection
    file_base = OutputData/MixedSesquilinear
    vtk_format = ASCII
  []
[]
