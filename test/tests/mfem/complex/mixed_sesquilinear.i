[Mesh]
  type = MFEMMesh
  file = ../mesh/square.e
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
  [HCurlFESpace]
    type = MFEMVectorFESpace
    fec_type = ND
    fec_order = FIRST
  []

[]

[Variables]
  [p]
    type = MFEMComplexVariable
    fespace = H1FESpace
  []
  [E]
    type = MFEMComplexVariable
    fespace = HCurlFESpace
  []
[]

[Functions]
  [f1_real]
    type = ParsedFunction
    expression = '-pi*cos(pi*x)*sin(pi*y)'
  []
  [f1_imag]
    type = ParsedFunction
    expression = '-sin(pi*x)*sin(pi*y)'
  []
  [f2_real]
    type = ParsedVectorFunction
    expression_x = '-pi*sin(pi*x)*cos(pi*y)'
    expression_y = 'pi*cos(pi*x)*sin(pi*y)'
  []
  [f2_imag]
    type = ParsedVectorFunction
    expression_x = '0.0'
    expression_y = '-sin(pi*x)*sin(pi*y)'
  []
  [p_exact]
    type = ParsedFunction
    expression = 'sin(pi*x)*sin(pi*y)'
  []
  [E_exact] type = ParsedVectorFunction 
    expression_x = '0.0' 
    expression_y = 'sin(pi*x)*sin(pi*y)' 
  []
[]

[BCs]
  [dbc_p]
    type = MFEMComplexScalarDirichletBC
    variable = p
    coefficient_real = 0.0
    coefficient_imag = 0.0
  []
  [dbc_E_tan]
    type = MFEMComplexVectorTangentialDirichletBC
    variable = E
    vector_coefficient_real = '0.0 0.0'
    vector_coefficient_imag = '0.0 0.0'
  []
  [dbc_E_norm]
    type = MFEMComplexVectorNormalDirichletBC
    variable = E
    vector_coefficient_real = '0.0 0.0'
    vector_coefficient_imag = '0.0 0.0'
  []

[]

[Kernels]
  [mass_p]
    type = MFEMComplexKernel
    variable = p
    [RealComponent]
      type = MFEMMassKernel
      coefficient = -1.0
    []
    [ImagComponent]
      type = MFEMMassKernel
      coefficient = 1.0
    []
  []
  [mass_E]
    type = MFEMComplexKernel
    variable = E
    [RealComponent]
      type = MFEMVectorFEMassKernel
      coefficient = 0.0
    []
    [ImagComponent]
      type = MFEMVectorFEMassKernel
      coefficient = 1.0
    []
  []
  [scalarcurl_E]
    type = MFEMMixedSesquilinearFormKernel
    trial_variable = E
    variable = p
    [RealComponent]
      type = MFEMMixedScalarCurlKernel
      coefficient = 1.0
    []
    [ImagComponent]
      type = MFEMMixedScalarCurlKernel
      coefficient = 0.0
    []
  []
  [scalarcurl_p]
    type = MFEMMixedSesquilinearFormKernel
    trial_variable = p
    variable = E
    [RealComponent]
      type = MFEMMixedScalarWeakCurlKernel
      coefficient = -1.0
    []
    [ImagComponent]
      type = MFEMMixedScalarWeakCurlKernel
      coefficient = 0.0
    []
  []
  [scalar_forcing]
    type = MFEMComplexKernel
    variable = p
    [RealComponent]
      type = MFEMDomainLFKernel
      coefficient = f1_real
    []
    [ImagComponent]
      type = MFEMDomainLFKernel
      coefficient = f1_imag
    []
  []
  [vector_forcing]
    type = MFEMComplexKernel
    variable = E
    [RealComponent]
      type = MFEMVectorFEDomainLFKernel
      vector_coefficient = f2_real
    []
    [ImagComponent]
      type = MFEMVectorFEDomainLFKernel
      vector_coefficient = f2_imag
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
