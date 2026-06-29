a=1.0
b=2.0
c=3.0
d=4.0
e=5.0
f=6.0
omega=10.0

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
    fec_order = SECOND
  []
  [HCurlFESpace]
    type = MFEMVectorFESpace
    fec_type = ND
    fec_order = SECOND
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
    expression = '${a}*x^2+${b}*y^2+${c}*z^2'
  []
  [V_exact_imag]
    type = ParsedFunction
    expression = '${d}*x^2+${e}*y^2+${f}*z^2'
  []
  [A_exact_real]
    type = ParsedVectorFunction
    expression_x = '-2*${d}*x/${omega}'
    expression_y = '-2*${e}*y/${omega}'
    expression_z = '-2*${f}*z/${omega}'
  []
  [A_exact_imag]
    type = ParsedVectorFunction
    expression_x = '2*${a}*x/${omega}'
    expression_y = '2*${b}*y/${omega}'
    expression_z = '2*${c}*z/${omega}'
  []
[]

[ICs]
  [A_vector_ic]
    type = MFEMComplexVectorIC
    variable = A
    vector_coefficient_real = A_exact_real
    vector_coefficient_imag = A_exact_imag
  []
[]

[BCs]
  [dbc_V]
    type = MFEMComplexScalarDirichletBC
    variable = V
    coefficient_real = V_exact_real
    coefficient_imag = V_exact_imag
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
  [mass_A]
    type = MFEMMixedSesquilinearFormKernel
    trial_variable = A
    variable = V
    [ImagComponent]
      type = MFEMMixedVectorWeakDivergenceKernel
      coefficient = -${omega}
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
  assembly_level = legacy
[]

[Postprocessors]
  [error]
    type = MFEMComplexL2Error
    variable = V
    function_real = V_exact_real
    function_imag = V_exact_imag
  []
[]

[Outputs]
  csv = true
  file_base = OutputData/MixedSesquilinearIC
[]
