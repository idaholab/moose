a=1.0
b=2.0
c=3.0
omega=10.0

[Mesh]
  type = MFEMMesh
  file = ../mesh/square.msh
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
  [HCurlFESpace]
    type = MFEMVectorFESpace
    fec_type = ND
    fec_order = FIRST
  []
[]

[Variables]
  [V]
    type = MFEMComplexVariable
    fespace = H1FESpace
  []
  [A]
    type = MFEMComplexVariable
    fespace = HCurlFESpace
  []
[]

[Functions]
  [V_exact_real]
    type = ParsedFunction
    expression = '${a}*x+${b}*y+${c}*z'
  []
  [A_exact_imag]
    type = ParsedVectorFunction
    expression_x = '${a}/${omega}'
    expression_y = '${b}/${omega}'
    expression_z = '${c}/${omega}'
  []
[]

[BCs]
  [dbc_V]
    type = MFEMComplexScalarDirichletBC
    variable = V
    coefficient_real = V_exact_real
    coefficient_imag = 0.0
  []
  [dbc_A_tan]
    type = MFEMComplexVectorTangentialDirichletBC
    variable = A
    vector_coefficient_real = '0 0 0'
    vector_coefficient_imag = A_exact_imag
  []
[]

[Kernels]
  [curlcurl_A]
    type = MFEMComplexKernel
    variable = A
    [RealComponent]
      type = MFEMCurlCurlKernel
      coefficient = 1.0
    []
  []
  [mixed_grad_V]
    type = MFEMMixedSesquilinearFormKernel
    trial_variable = V
    variable = A
    [RealComponent]
      type = MFEMMixedVectorGradientKernel
      coefficient = 1.0
    []
  []
  [mass_A]
    type = MFEMComplexKernel
    variable = A
    [ImagComponent]
      type = MFEMVectorFEMassKernel
      coefficient = ${omega}
    []
  []
  [diff_V]
    type = MFEMComplexKernel
    variable = V
    [RealComponent]
      type = MFEMDiffusionKernel
      coefficient = 1.0
    []
  []
  [mixed_mass_A]
    type = MFEMMixedSesquilinearFormKernel
    trial_variable = A
    variable = V
    [ImagComponent]
      type = MFEMMixedVectorWeakDivergenceKernel
      coefficient = -${omega}
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

[Postprocessors]
  [error_V]
    type = MFEMComplexL2Error
    variable = V
    function_real = V_exact_real
    function_imag = 0.0
  []
  [error_A]
    type = MFEMComplexVectorL2Error
    variable = A
    function_real = '0 0 0'
    function_imag = A_exact_imag
  []
[]

[Outputs]
  csv = true
  file_base = OutputData/MixedSesquilinear
[]
