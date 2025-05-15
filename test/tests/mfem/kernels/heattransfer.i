[Mesh]
  type = MFEMMesh
  file = ../mesh/mug.e
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

[Functions]
  [reservoir_far_temperature]
    type = ParsedFunction
    expression = 0.5
  []
  [heat_transfer_coefficient]
    type = ParsedFunction
    expression = 5.0
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

[BCs]
  active = 'bottom top_convective'
  [bottom]
    type = MFEMScalarDirichletBC
    variable = temperature
    boundary = '1'
    value = 1.0
  []
  [top_convective]
    type = MFEMConvectiveHeatFluxBC
    variable = temperature
    boundary = '2'
    T_infinity = reservoir_far_temperature
    heat_transfer_coefficient = heat_transfer_coefficient
  []
  [top_dirichlet]
    type = MFEMScalarDirichletBC
    variable = temperature
    boundary = '2'
    value = 0.0
  []
[]

[FunctorMaterials]
  [Substance]
    type = MFEMGenericConstantFunctorMaterial
    prop_names = 'thermal_conductivity volumetric_heat_capacity'
    prop_values = '1.0 1.0'
  []
[]

[Preconditioner]
  [boomeramg]
    type = MFEMHypreBoomerAMG
  []
  [jacobi]
    type = MFEMOperatorJacobiSmoother
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
  device = cpu
  assembly_level = legacy
  dt = 2.0
  start_time = 0.0
  end_time = 6.0
[]

[Outputs]
  [ParaViewDataCollection]
    type = MFEMParaViewDataCollection
    file_base = OutputData/HeatTransfer
    vtk_format = ASCII
  []
[]
