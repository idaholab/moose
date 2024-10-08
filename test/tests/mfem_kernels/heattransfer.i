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
  [temperature]
    type = MFEMVariable
    fespace = H1FESpace
  []
[]

[Functions]
  [value_bottom]
    type = ParsedFunction
    expression = 1.0
  []
[]

[Coefficients]
  [BottomValue]
    type = MFEMFunctionCoefficient
    function = value_bottom
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
  [bottom]
    type = MFEMScalarDirichletBC
    variable = temperature
    boundary = '1'
    coefficient = BottomValue
  []
  [top]
    type = MFEMConvectiveHeatFluxBC
    variable = temperature
    boundary = '2'
    T_infinity = 'reservoir_far_temperature'
    heat_transfer_coefficient = 'heat_transfer_coefficient'
  []
[]

[Materials]
  [Substance]
    type = MFEMGenericConstantMaterial
    prop_names = 'thermal_conductivity volumetric_heat_capacity'
    prop_values = '1.0 1.0'
  []
  [Boundary]
    type = MFEMGenericConstantMaterial
    prop_names = 'heat_transfer_coefficient reservoir_far_temperature'
    prop_values = '5.0 0.5'
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
  device = cpu
  dt = 2.0
  start_time = 0.0
  end_time = 10.0
[]

[Outputs]
  [ParaViewDataCollection]
    type = MFEMParaViewDataCollection
    file_base = OutputData/HeatTransfer
    vtk_format = ASCII
  []
[]
