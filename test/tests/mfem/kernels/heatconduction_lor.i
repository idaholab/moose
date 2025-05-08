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
    type = MFEMScalarFESpace
    fec_type = H1
    fec_order = SECOND
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
  [hyprefgmres]
    type = MFEMHypreFGMRES
    low_order_refined = true
    l_tol = 1e-22
    l_max_its = 2000  
  []
[]

[Solver]
  type = MFEMCGSolver
  preconditioner = hyprefgmres
  l_tol = 1e-22
  l_max_its = 2000
[]

[Executioner]
  type = MFEMTransient
  device = cpu
  dt = 0.25
  start_time = 0.0
  end_time = 1.0
[]

[Outputs]
  [ParaViewDataCollection]
    type = MFEMParaViewDataCollection
    file_base = OutputData/HeatConductionLOR
    vtk_format = ASCII
  []
[]
