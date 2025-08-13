[Mesh]
  type = MFEMMesh
  file = ../mesh/ermes_mouse_coarse.g
  dim = 3
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
  [HDivFESpace]
    type = MFEMVectorFESpace
    fec_type = RT
    fec_order = CONSTANT
  []
[]

[Variables]
  [E]
    type = MFEMComplexVariable
    fespace = HCurlFESpace
  []
 #[J]
 #  type = MFEMComplexVariable
 #  fespace = HDivFESpace
 #[]
[]

[Functions]
  [mu0]
    type = ParsedFunction
    expression = 4.0e-7*pi
    symbol_names = 'pi'
    symbol_values = 3.1415926535897932384
  []
  [epsilon0]
    type = ParsedFunction
    expression = 8.8541878176e-12
  []
  [freq]
    type = ParsedFunction
    expression = 9e8
  []
  [RWTE10Real]
    type = ParsedFunction
    expression = 0.0 # NOT YET SET
  []
  [RWTE10Imag]
    type = ParsedFunction
    expression = 0.0 # NOT YET SET
  []
  [massCoef]
    type = ParsedFunction
    expression = 0.0 # NOT YET SET # k_imag / mu0_
  []
  [stiffnessCoef]
    type = ParsedFunction
    expression = 0.0 # NOT YET SET
  []
  [lossCoef]
    type = ParsedFunction
    expression = 0.0 # NOT YET SET
  []
  [vecZero]
    type = ParsedVectorFunction
    expression_x = 0.0
    expression_y = 0.0
    expression_z = 0.0
  []
  [port_length_vector]
    type = ParsedVectorFunction
    expression_x = 24.76e-2
    expression_y = 0.0
    expression_z = 0.0
  []
  [port_width_vector]
    type = ParsedVectorFunction
    expression_x = 0.0
    expression_y = 12.38e-2
    expression_z = 0.0
  []
[]

[FunctorMaterials]
  [Mouse]
    type = MFEMGenericFunctorMaterial
    prop_names = 'elec_conductivity dielec_permittivity mag_permeability'
    prop_values = '0.0 epsilon0 mu0'
    block = 1
  []
  [Air]
    type = MFEMGenericFunctorMaterial
    prop_names = 'elec_conductivity dielec_permittivity mag_permeability'
    prop_values = '0.97 43 * epsilon0 mu0'
    block = 2
  []
[]

[BCs]
  [tangential_E]
    type = MFEMComplexVectorTangentialDirichletBC
    variable = E
    boundary = '2 3 4'
    coefficient_real = vecZero
    coefficient_imag = vecZero
  []
  [WaveguidePortInLF]
    type = MFEMComplexIntegratedBC
    variable = E
    boundary = '5'
    [real_part]
      type = MFEMVectorFEBoundaryTangentIntegratedBC
      coefficient = RWTE10Real
    []
    [imag_part]
      type = MFEMVectorFEBoundaryTangentIntegratedBC
      coefficient = RWTE10Imag
    []
  []
  [WaveguidePortInBF]
    type = MFEMComplexIntegratedBC
    variable = E
    boundary = '5'
    [real_part]
      type = MFEMVectorFEMassIntegratedBC
      coefficient = 0.0
    []
    [imag_part]
      type = MFEMVectorFEMassIntegratedBC
      coefficient = massCoef
    []
  []
  [WaveguidePortOutBF]
    type = MFEMVectorFEBoundaryTangentIntegratedBC
    variable = E
    boundary = '6'
    [real_part]
      type = MFEMVectorFEMassIntegratedBC
      coefficient = 0.0
    []
    [imag_part]
      type = MFEMVectorFEMassIntegratedBC
      coefficient = massCoef
    []
  []
[]

[Kernels]
  [curlcurl]
    type = MFEMComplexKernel
    variable = E
    [real_part]
      type = MFEMCurlCurlKernel
      coefficient = stiffnessCoef
    []
    [imag_part]
      type = MFEMCurlCurlKernel
      coefficient = 0.0
    []
  []
  [mass_loss]
    type = MFEMComplexKernel
    variable = E
    [real_part]
      type = MFEMVectorFEMassKernel
      coefficient = massCoef
    []
    [imag_part]
      type = MFEMVectorFEMassKernel
      coefficient = lossCoef
    []
  []
[]

[Solver]
  type = MFEMSuperLU
[]

[Executioner]
  type = MFEMSteady
  numeric_type = complex
[]

[Outputs]
  [ParaViewDataCollection]
    type = MFEMParaViewDataCollection
    file_base = OutputData/Complex
    vtk_format = ASCII
  []
[]
