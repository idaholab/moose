mu = 1.0
epsilon = 1.0
sigma = 20.0
omega = 10.0
kappa_r = 12.7201964951406889525742371916
kappa_i = -7.86151377757423297509831172647

[Mesh]
  type = MFEMMesh
  file = ../mesh/inline-quad.mesh
  dim = 3
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
    boundary = '1 2 3 4'
    coefficient_real = u0_r
    coefficient_imag = u0_i
  []
[]

[Kernels]
  [diffusion_complex]
    type = MFEMComplexKernel
    variable = u
    [real_part]
      type = MFEMDiffusionKernel
      coefficient = stiffnessCoef
    []
    [imag_part]
      type = MFEMDiffusionKernel
      coefficient = 0.0
    []
  []
  [mass_complex]
    type = MFEMComplexKernel
    variable = u
    [real_part]
      type = MFEMMassKernel
      coefficient = massCoef
    []
    [imag_part]
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
    file_base = OutputData/Complex
    vtk_format = ASCII
  []
[]
