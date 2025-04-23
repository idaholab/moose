[Mesh]
  type = MFEMMesh
  file = ../../../../unit/data/mug.e
  dim = 3
[]

[Problem]
  type = MFEMProblem
[]

[FESpaces]
  [H1FESpace]
    type = MFEMScalarFESpace
    fec_type = H1
    fec_order = FIRST
  []
[]

[Variables]
  [temperature]
    type = MFEMVariable
    fespace = H1FESpace
  []
[]

[BCs]
  [bottom]
    type = MFEMScalarDirichletBC
    variable = temperature
    boundary = '1'
    value = 1.0
  []
  [low_terminal]
    type = MFEMScalarDirichletBC
    variable = temperature
    boundary = '2'
    value = 0.0
  []
[]

[Materials]
  [Substance]
    type = MFEMGenericConstantMaterial
    prop_names = 'thermal_conductivity volumetric_heat_capacity'
    prop_values = '1.0 1.0'
  []
[]

[Kernels]
  [diff]
    type = MFEMDiffusionKernel
    variable = temperature
    coefficient = thermal_conductivity
  []
  [dT_dt]
    type = MFEMTimeDerivativeMassKernel
    variable = temperature
    coefficient = volumetric_heat_capacity
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
[]

[Executioner]
  type = MFEMTransient
  device = cpu
  dt = 0.25
  start_time = 0.0
  end_time = 1.0
  assembly_level = element
[]

[Outputs]
  [ParaViewDataCollection]
    type = MFEMParaViewDataCollection
    file_base = OutputData/HeatConductionElement
    vtk_format = ASCII
  []
[]
