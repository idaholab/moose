[Mesh]
  type = MFEMMesh
  file = gold/mug.e
  dim = 3
[]

[Problem]
  type = MFEMProblem
  device = "cpu"
[]

[Formulation]
  type = TransientCustomFormulation
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

[Coefficients]
  [one]
    type = MFEMConstantCoefficient
    value = 1.0
  []
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
    coefficient = one
  []
[]

[Executioner]
  type = Transient
  dt = 1.0
  start_time = 0.0
  end_time = 1.0

  l_tol = 1e-16
  l_max_its = 1000
[]

[Outputs]
  [ParaViewDataCollection]
    type = MFEMParaViewDataCollection
    file_base = OutputData/VariableSetupTest
  []
[]
