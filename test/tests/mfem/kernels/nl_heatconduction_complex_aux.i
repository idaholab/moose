# Transient nonlinear heat conduction with a complex auxiliary variable.
# The complex aux variable exercises the cmplx_gridfunctions checkpoint/recover code path
# in SolutionStateData.C without requiring a complex primary solve.

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
  [complex_temperature]
    type = MFEMComplexVariable
    fespace = H1FESpace
  []
[]

[AuxKernels]
  [project_complex_temperature]
    type = MFEMComplexScalarProjectionAux
    variable = complex_temperature
    coefficient_real = temperature
    coefficient_imag = temperature
    execute_on = TIMESTEP_END
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

[Solver]
  type = MFEMMUMPS
  print_level = 0
[]

[Executioner]
  type = MFEMTransient
  device = cpu
  assembly_level = legacy
  dt = 1e-2
  start_time = 0.0
  end_time = 0.5

  nl_max_its = 30
  nl_abs_tol = 1.0e-5
  nl_rel_tol = 1.0e-5
  print_level = 1
[]

[VectorPostprocessors]
  [centre_temperature]
    type = MFEMPointValueSampler
    variable = 'temperature'
    points = '0.0 0.0 0.0'
    execute_on = TIMESTEP_END
  []
  [centre_complex_temperature]
    type = MFEMComplexPointValueSampler
    variable = 'complex_temperature'
    points = '0.0 0.0 0.0'
    execute_on = TIMESTEP_END
  []
[]

[Outputs]
  file_base = OutputData/NLHeatConductionComplexAux
  csv = true
  time_step_interval = 10
[]
