[Mesh]
  type = MFEMMesh
  file = ../mesh/square.e
[]

[Problem]
  type = MFEMProblem
[]

[FESpaces]
  [HDivFESpace]
    type = MFEMVectorFESpace
    fec_type = RT
    fec_order = FIRST
  []
  [L2FESpace]
    type = MFEMScalarFESpace
    fec_type = L2
    fec_order = CONSTANT
  []
[]

[Variables]
  [time_integrated_heat_flux]
    type = MFEMVariable
    fespace = HDivFESpace
    time_derivative = heat_flux
  []
  [temperature]
    type = MFEMVariable
    fespace = L2FESpace
  []
[]

[Kernels]
  [dT_dt,T']
    type = MFEMTimeDerivativeMassKernel
    variable = temperature
  []
  [divh,T']
    type = MFEMVectorFEDivergenceKernel
    trial_variable = heat_flux
    variable = temperature
  []
  [h,h']
    type = MFEMTimeDerivativeVectorFEMassKernel
    variable = time_integrated_heat_flux
  []
  [-T,div.h']
    type = MFEMVectorFEDivergenceKernel
    trial_variable = temperature
    variable = time_integrated_heat_flux
    coefficient = -1.0
    transpose = true
  []
[]

[BCs]
  [right]
    type = MFEMVectorFEBoundaryFluxIntegratedBC
    variable = time_integrated_heat_flux
    coefficient = 0.0
    boundary = 2
  []
  [left]
    type = MFEMVectorFEBoundaryFluxIntegratedBC
    variable = time_integrated_heat_flux
    coefficient = -1.0
    boundary = 4
  []
  [topbottom]
    type = MFEMVectorDirichletBC
    variable = heat_flux
    vector_coefficient = '0.0 0.0'
    boundary = '1 3'
  []
[]

[Solver]
  type = MFEMSuperLU
[]

[Executioner]
  type = MFEMTransient
  device = cpu
  assembly_level = legacy
  dt = 0.03
  start_time = 0.0
  end_time = 0.09
[]

[Outputs]
  [ParaViewDataCollection]
    type = MFEMParaViewDataCollection
    file_base = OutputData/MixedHeatTransfer
    vtk_format = ASCII
  []
[]
