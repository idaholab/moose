# Nonlinear heat conduction (MFEM Example 16), with the temperature-dependent diffusivity routed
# through a scalar quadrature function coefficient. The stored values are re-projected on each
# nonlinear iteration, reproducing the reference solve in nlheatconduction.i.

kappa = 0.5
alpha = 1e-2

[Mesh]
  type = MFEMFileMesh
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

[QuadratureFunctions]
  [qf_k]
    type = MFEMScalarQuadratureFunction
    coefficient = diffusivity_temperature_dependence
    # the quadrature rule order matches the one used by DiffusionIntegrator for
    # second-order H1 elements on quadrilaterals (2 * fe_order + dim - 1 = 5)
    order = 5
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
    k_coefficient = qf_k
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
  file_base = NLHeatConductionQF
  csv = true
  time_step_interval = 10

  [ParaViewDataCollection]
    type = MFEMParaViewDataCollection
    file_base = OutputData/NLHeatConductionQF
    vtk_format = ASCII
  []
[]
