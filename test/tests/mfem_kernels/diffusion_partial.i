[Mesh]
  type = MFEMMesh
  file = gold/mug.e
  dim = 3
[]

[Problem]
  type = MFEMProblem
[]

[FESpaces]
  [H1FESpace]
    type = MFEMFESpace
    fec_type = H1
    fec_order = FIRST
  []
[]

[Variables]
  [diffused]
    type = MFEMVariable
    fespace = H1FESpace
  []
[]

[Functions]
  [value_bottom]
    type = ParsedFunction
    expression = 1.0
  []
  [value_top]
    type = ParsedFunction
    expression = 0.0
  []
[]

[BCs]
  [bottom]
    type = MFEMScalarDirichletBC
    variable = diffused
    boundary = '1'
    coefficient = BottomValue
  []
  [low_terminal]
    type = MFEMScalarDirichletBC
    variable = diffused
    boundary = '2'
    coefficient = TopValue
  []
[]

[Materials]
  [Substance]
    type = MFEMGenericConstantMaterial
    prop_names = diffusivity
    prop_values = 1.0
  []
[]

[Coefficients]
  [TopValue]
    type = MFEMFunctionCoefficient
    function = value_top
  []
  [BottomValue]
    type = MFEMFunctionCoefficient
    function = value_bottom
  []
[]

[Kernels]
  [diff]
    type = MFEMDiffusionKernel
    variable = diffused
    coefficient = diffusivity
  []
[]

[Preconditioner]
  [jacobi]
    type = MFEMOperatorJacobiSmoother
  []
[]

[Solver]
  type = MFEMCGSolver
  preconditioner = jacobi
  l_tol = 1e-16
  l_max_its = 1000
  print_level = 2
[]

[Executioner]
  type = MFEMSteady
  assembly_level = partial
[]

[Outputs]
  [ParaViewDataCollection]
    type = MFEMParaViewDataCollection
    file_base = OutputData/DiffusionPartial
    vtk_format = ASCII
  []
[]
