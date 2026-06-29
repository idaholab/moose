a=1.0
b=2.0
c=3.0
d=4.0
e=5.0
f=6.0
omega=10.0
alpha=1.0
beta=0.5

[Mesh]
  type = MFEMMesh
  file = ../mesh/square.msh
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
  [V_exact_imag]
    type = ParsedFunction
    expression = '${d}*x+${e}*y+${f}*z'
  []
  [A_exact_real]
    type = ParsedVectorFunction
    expression_x = '-(${beta}*${a}+${alpha}*${d})/${omega}'
    expression_y = '-(${beta}*${b}+${alpha}*${e})/${omega}'
    expression_z = '-(${beta}*${c}+${alpha}*${f})/${omega}'
  []
  [A_exact_imag]
    type = ParsedVectorFunction
    expression_x = '(${alpha}*${a}-${beta}*${d})/${omega}'
    expression_y = '(${alpha}*${b}-${beta}*${e})/${omega}'
    expression_z = '(${alpha}*${c}-${beta}*${f})/${omega}'
  []
[]

[BCs]
  [dbc_V]
    type = MFEMComplexScalarDirichletBC
    variable = V
    coefficient_real = V_exact_real
    coefficient_imag = V_exact_imag
  []
  [dbc_A_tan]
    type = MFEMComplexVectorTangentialDirichletBC
    variable = A
    vector_coefficient_real = A_exact_real
    vector_coefficient_imag = A_exact_imag
  []
[]

[Kernels]
  [diff_V]
    type = MFEMComplexKernel
    variable = V
    [RealComponent]
      type = MFEMDiffusionKernel
      coefficient = 1.0
    []
  []
  [curlcurl_A]
    type = MFEMComplexKernel
    variable = A
    [RealComponent]
      type = MFEMCurlCurlKernel
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
  [mixed_grad_V]
    type = MFEMMixedSesquilinearFormKernel
    trial_variable = V
    variable = A
    [RealComponent]
      type = MFEMMixedVectorGradientKernel
      coefficient = ${alpha}
    []
    [ImagComponent]
      type = MFEMMixedVectorGradientKernel
      coefficient = ${beta}
    []
  []
[]

[Solvers]
  [main]
    type = MFEMSuperLU
  []
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
    function_imag = V_exact_imag
  []
  [error_A]
    type = MFEMComplexVectorL2Error
    variable = A
    function_real = A_exact_real
    function_imag = A_exact_imag
  []
[]

[Outputs]
  csv = true
  file_base = OutputData/MixedSesquilinearRealImag
[]
