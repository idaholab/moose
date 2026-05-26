# Implementation of MFEM Example 16, for a time dependent nonlinear heat equation problem of the
# form
# dT/dt = \nabla \cdot (\kappa + \alpha T) \nabla T

kappa = 0.5
alpha = 1e-2

[Mesh]
  type = MFEMMesh
  file = ../mesh/star.mesh
  uniform_refine = 1
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

[AuxVariables]
  inactive = average_temperature
  [average_temperature]
    type = MFEMVariable
    fespace = H1FESpace
  []
[]

[AuxKernels]
  inactive = average_field
  [average_field]
    type = MFEMScalarTimeAverageAux
    variable = average_temperature
    source = temperature
  []
[]

[Functions]
  [initial]
    type = ParsedFunction
    expression = 'if((x*x + y*y > 0.251), 1.0, 2.0)'
  []
  [diffusivity_temperature_dependence]
    type = MFEMParsedFunction
    expression = 'alpha * temperature'
    symbol_names = 'alpha temperature'
    symbol_values = '${alpha} temperature'
  []
[]

[ICs]
  [diffused_ic]
    type = MFEMScalarIC
    coefficient = initial
    variable = temperature
  []
[]

[Solvers]
  [nl]
    type = MFEMNewtonNonlinearSolver
    max_its = 30
    abs_tol = 1.0e-5
    rel_tol = 1.0e-5
    print_level = 1
  []
  [main]
    type = MFEMMUMPS
    print_level = 0
  []
[]

[Kernels]
  [nl_diffusion]
    type = MFEMNLDiffusionKernel
    variable = temperature
    k_coefficient = diffusivity_temperature_dependence
    dk_du_coefficient = ${alpha}
  []
  [linear_diffusion]
    type = MFEMDiffusionKernel
    variable = temperature
    coefficient = ${kappa}
  []
  [dT_dt]
    type = MFEMTimeDerivativeMassKernel
    variable = temperature
  []
[]

[Executioner]
  type = MFEMTransient
  device = cpu
  assembly_level = legacy
  dt = 1e-2
  start_time = 0.0
  end_time = 0.5
[]

[VectorPostprocessors]
  [centre_temperature]
    type = MFEMPointValueSampler
    variable = 'temperature'
    points = '0.0 0.0 0.0'
    execute_on = TIMESTEP_END
  []
[]

[Outputs]
  file_base = OutputData/NLHeatConduction
  csv = true
  time_step_interval = 10

  [ParaViewDataCollection]
    type = MFEMParaViewDataCollection
    file_base = OutputData/NLHeatConduction
    vtk_format = ASCII
  []
[]
