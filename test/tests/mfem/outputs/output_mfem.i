[Mesh]
  type = CoupledMFEMMesh
  file = gold/simple-cube-tet10.e
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
    type = MFEMConstantCoefficient
    value = 0
  []
  [BottomValue]
    type = MFEMConstantCoefficient
    value = 1
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
  dt = 0.1
  start_time = 0.0
  end_time = 0.1

  l_tol = 1e-5
  l_max_its = 1000
[]

[Outputs]
  [ParaViewDataCollection]
    type = MFEMParaViewDataCollection
    file_base = OutputData/ParaViewCoil
  []
[]
