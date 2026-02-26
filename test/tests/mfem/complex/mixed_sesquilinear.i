a=1.0
b=1.0
c=1.0
omega=1.0

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

[ICs]
  [A_vector_ic]
    type = MFEMComplexVectorIC
    variable = A
    vector_coefficient_real = '0 0 0'
    vector_coefficient_imag = A_exact_imag
  []
[]

[BCs]
  [dbc_V]
    type = MFEMComplexScalarDirichletBC
    variable = V
    coefficient_real = V_exact_real
    coefficient_imag = 0.0
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
