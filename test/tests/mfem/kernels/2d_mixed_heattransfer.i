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
    fec_order = SECOND
  []
  [L2FESpace]
    type = MFEMScalarFESpace
    fec_type = L2
    fec_order = FIRST
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

[AuxVariables]
  [div_heat_flux]
    type = MFEMVariable
    fespace = L2FESpace
  []  
[]

[AuxKernels]
  [div_heat_flux]
    type = MFEMDivAux
    variable = div_heat_flux
    source = heat_flux
    execute_on = TIMESTEP_END
  []
[]

[Kernels]
  [h,h']
    type = MFEMTimeDerivativeVectorFEMassKernel
    variable = time_integrated_heat_flux
  []
  [-dT_dt,div.h'] # Shouldn't be necessary, as should be part of -T,div.h'
    type = MFEMMixedScalarDivergenceKernel
    trial_variable = dtemperature_dt
    # trial_variable = temperature
    variable = time_integrated_heat_flux
    coefficient = -1.0
    transpose = true
  []
  [-T,div.h']
    type = MFEMMixedScalarDivergenceKernel
    # trial_variable = dtemperature_dt
    trial_variable = temperature
    variable = time_integrated_heat_flux
    coefficient = -1.0
    transpose = true
  []

  [dT_dt,dT_dt']
    type = MFEMTimeDerivativeMassKernel
    variable = temperature
    coefficient = 1.0
  []  
  [divh,dT_dt']
    type = MFEMMixedScalarDivergenceKernel
    trial_variable = heat_flux
    variable = temperature
    coefficient = 1.0
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
  # [right]
  #   type = MFEMVectorNormalDirichletBC
  #   variable = heat_flux
  #   vector_coefficient = '-0.1 0.0'
  #   boundary = 2
  # []
  # [left]
  #   type = MFEMVectorNormalDirichletBC
  #   variable = heat_flux
  #   vector_coefficient = '-0.1 0.0'
  #   boundary = 4
  # []
  # [bottom]
  #   type = MFEMVectorNormalDirichletBC
  #   variable = time_integrated_heat_flux
  #   vector_coefficient = '0.0 0.0 -0.1'
  #   boundary = 1
  # []
  # [top]
  #   type = MFEMVectorNormalDirichletBC
  #   variable = time_integrated_heat_flux
  #   vector_coefficient = '0.0 0.0 0.2'
  #   boundary = 2
  # []
  # [bottom]
  #   type = MFEMVectorFEBoundaryFluxIntegratedBC
  #   variable = time_integrated_heat_flux
  #   boundary = '1'
  #   coefficient = 1.0
  # []
  # [top]
  #   type = MFEMVectorFEBoundaryFluxIntegratedBC
  #   variable = time_integrated_heat_flux
  #   coefficient = -1.0
  #   boundary = 2
  # []
[]

[Solver]
  type = MFEMSuperLU
[]

[Executioner]
  type = MFEMTransient
  device = cpu
  assembly_level = legacy
  dt = 1.0
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
