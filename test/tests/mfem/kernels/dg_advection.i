# Solves du/dt + Q.grad(u) = 0 for a rotating velocity Q with zero inflow, discretized with the
# non-conservative upwind DG method and backward Euler time integration, and records the L2 norm
# of the solution at every step. The face terms use their default upwind weights alpha = 1 and
# beta = 1/2, and the inflow term the matching alpha = -1 and beta = -1/2 on the right hand side.

[Mesh]
  type = MFEMFileMesh
  file = ../mesh/ref-square.mesh
  uniform_refine = 4
[]

[Problem]
  type = MFEMProblem
[]

[FESpaces]
  [L2FESpace]
    type = MFEMScalarFESpace
    fec_type = L2
    fec_order = THIRD
    basis = GaussLobatto
  []
[]

[Variables]
  [u]
    type = MFEMVariable
    fespace = L2FESpace
  []
[]

[Functions]
  # The velocity and initial condition are written in the coordinates X = 2 x - 1, Y = 2 y - 1,
  # which map the unit square to [-1, 1]^2.
  # Clockwise rotation about the centre of the square
  [velocity]
    type = ParsedVectorFunction
    expression_x = 'pi / 2 * (2 * y - 1)'
    expression_y = '-pi / 2 * (2 * x - 1)'
  []
  # sin^2(pi rho) sin(3 phi) in polar coordinates, which is nonzero on the inflow boundary
  [u0]
    type = ParsedFunction
    expression = 'sin(pi * hypot(2 * x - 1, 2 * y - 1))^2 * sin(3 * atan2(2 * y - 1, 2 * x - 1))'
  []
[]

[ICs]
  [u0]
    type = MFEMScalarIC
    variable = u
    coefficient = u0
  []
[]

[Kernels]
  [du_dt]
    type = MFEMTimeDerivativeMassKernel
    variable = u
  []
  [convection]
    type = MFEMConvectionKernel
    variable = u
    vector_coefficient = velocity
  []
  [dg_trace]
    type = MFEMNonconservativeDGTraceKernel
    variable = u
    vector_coefficient = velocity
  []
[]

[BCs]
  [dg_trace]
    type = MFEMNonconservativeDGTraceIntegratedBC
    variable = u
    vector_coefficient = velocity
  []
  [inflow]
    type = MFEMBoundaryFlowIntegratedBC
    variable = u
    coefficient = 0
    vector_coefficient = velocity
  []
[]

[Solvers]
  [jacobi]
    type = MFEMOperatorJacobiSmoother
  []
  [main]
    type = MFEMGMRESSolver
    preconditioner = jacobi
    l_tol = 1e-12
    l_max_its = 1000
    print_level = 0
  []
[]

[Executioner]
  type = MFEMTransient
  device = cpu
  assembly_level = legacy
  dt = 0.01
  start_time = 0.0
  end_time = 0.5
[]

[Postprocessors]
  [l2_norm]
    type = MFEML2Error
    variable = u
    function = 0
    execute_on = 'initial timestep_end'
  []
[]

[Outputs]
  [csv]
    type = CSV
    file_base = OutputData/DGAdvection/dg_advection
  []
[]
