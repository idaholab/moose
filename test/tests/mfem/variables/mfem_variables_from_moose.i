[Mesh]
  type = MFEMMesh
  file = gold/mug.e
  dim = 3
[]

[Problem]
  type = MFEMProblem
  device = "cpu"
[]

[Variables]
  [scalar_var]
    family = LAGRANGE
    order = FIRST
  []
  [vector_var]
    family = LAGRANGE_VEC
    order = FIRST
  []  
[]

[AuxVariables]
  [scalar_auxvar]
    family = MONOMIAL
    order = CONSTANT
  []
  [vector_auxvar]
    family = MONOMIAL_VEC
    order = CONSTANT
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
    variable = scalar_var
    boundary = '1'
    coefficient = BottomValue
  []
  [low_terminal]
    type = MFEMScalarDirichletBC
    variable = scalar_var
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
    variable = scalar_var
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
  type = MFEMTransient
  dt = 1.0
  start_time = 0.0
  end_time = 1.0
[]

[Outputs]
  [ParaViewDataCollection]
    type = MFEMParaViewDataCollection
    file_base = OutputData/VariableSetupTest
  []
[]
