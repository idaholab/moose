[Mesh]
  type = CoupledMFEMMesh
  file = gold/mug.e
  dim = 3
[]

[Problem]
  type = MFEMProblem
  use_glvis = true
[]

[Formulation]
  type = CustomFormulation
[]

[AuxVariables]
  [mfem_diffused]
    family = LAGRANGE
    order = FIRST
  []
[]

[Functions]
  [value_bottom]
    type = ParsedFunction
    value = 1.0
  []
  [value_top]
    type = ParsedFunction
    value = 0.0
  []
[]

[BCs]
  [bottom]
    type = MFEMScalarDirichletBC
    variable = mfem_diffused
    boundary = '1'
    coefficient = BottomValue
  []
  [low_terminal]
    type = MFEMScalarDirichletBC
    variable = mfem_diffused
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
    variable = mfem_diffused
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
  [VisItDataCollection]
    type = MFEMVisItDataCollection
    file_base = OutputData/Diffusion
  []
[]
