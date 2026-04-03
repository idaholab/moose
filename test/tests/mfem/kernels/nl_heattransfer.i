[Problem]
  type = MFEMProblem
[]

[Mesh]
  type = MFEMMesh
  file = ../mesh/stacked_hexes.e
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

[ICs]
  [temperature_ic]
    type = MFEMScalarIC
    coefficient = 200.0
    variable = temperature
  []
[]

[Functions]
  [T_inf]
    type = MFEMParsedFunction
    expression = 'temperature + 1'
    symbol_names = 'temperature'
    symbol_values = 'temperature'
  []
  [htc]
    type = MFEMParsedFunction
    expression = 'temperature/100 + 1'
    symbol_names = 'temperature'
    symbol_values = 'temperature'
  []
  [dhtc_dT]
    type = MFEMParsedFunction
    expression = '1 / 100'
    symbol_names = 'temperature'
    symbol_values = 'temperature'
  []
[]

[Solvers]
  [nl]
    type = MFEMNewtonNonlinearSolver
    max_its = 150
    abs_tol = 1e-12
    rel_tol = 1.0e-8
    print_level = 1
  []
  [linear]
     type = MFEMMUMPS
     print_level = 0
  []
[]

[Kernels]
  [dT_dt]
    type = MFEMTimeDerivativeMassKernel
    variable = temperature
  []
  [diffusion]
    type = MFEMDiffusionKernel
    variable = temperature
  []
[]

[BCs]
  active = nonlinear
  [nonlinear]
    type = MFEMNLConvectiveHeatFluxBC
    variable = temperature
    boundary = 'right'
    T_infinity = T_inf
    heat_transfer_coefficient = htc
    d_heat_transfer_dT_coefficient = dhtc_dT
  []
  [linearized]
    type = MFEMNLConvectiveHeatFluxBC
    variable = temperature
    boundary = 'right'
    T_infinity = 201.0
    heat_transfer_coefficient = 3.0
    d_heat_transfer_dT_coefficient = 0.0
  []
[]

[VectorPostprocessors]
  [line_sample]
    type = MFEMLineValueSampler
    variable = 'temperature'
    start_point = '0.0 0.5 0.5'
    end_point = '1.0 0.5 0.5'
    num_points = 3
    execute_on = TIMESTEP_END
  []
[]

[Executioner]
  type = MFEMTransient
  device = cpu
  assembly_level = legacy
  dt = 1
  num_steps = 3
[]

[Outputs]
  [ParaViewDataCollection]
    type = MFEMParaViewDataCollection
    file_base = OutputData/NLHeatTransfer
    vtk_format = ASCII
  []
  [CSV]
    type = CSV
    file_base = OutputData/NLHeatTransfer
    time_step_interval = 3
  []
[]
