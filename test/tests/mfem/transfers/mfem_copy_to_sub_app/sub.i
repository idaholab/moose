[Mesh]
  type = MFEMMesh
  file = mug.e
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
  [sub]
    type = MFEMVariable
    fespace = H1FESpace
    ic = -10.0
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

[BCs]
  [bottom]
    type = MFEMScalarDirichletBC
    variable = sub
    boundary = '1'
    coefficient = BottomValue
  []
  [low_terminal]
    type = MFEMScalarDirichletBC
    variable = sub
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


[Kernels]
  [diff]
    type = MFEMDiffusionKernel
    variable = sub
    coefficient = diffusivity
  []
[]

[Preconditioner]
  [boomeramg]
    type = MFEMHypreBoomerAMG
  []
[]

[Solver]
  type = MFEMHypreGMRES
  preconditioner = boomeramg
  l_tol = 1e-16
  l_max_its = 1000  
[]


[Executioner]
  type = MFEMSteady
  device = cpu
[]

[Outputs]
  [ParaViewDataCollection]
    type = MFEMParaViewDataCollection
    file_base = OutputData/Diffusion_sub
    vtk_format = ASCII
  []
[]
