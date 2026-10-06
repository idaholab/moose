# Vibration of a cantilever beam after a gravitational load is switched on at t = 0.
#
# The second order elastodynamic equation rho d^2u/dt^2 = div(sigma(u)) + f is solved as
# a first order system in the displacement u and velocity v,
#
#   (rho dv/dt, w) + (sigma(u), grad(w)) = (f, w),
#   (du/dt, z) - (v, z) = 0,
#
# where both u and v are vector H1 fields. The momentum equation is tested with the
# displacement test functions so that the elasticity operator sits on the diagonal block.

[Mesh]
  type = MFEMFileMesh
  file = ../mesh/beam-tet.mesh
  uniform_refine = 1
[]

[Problem]
  type = MFEMProblem
[]

[FESpaces]
  [H1FESpace]
    type = MFEMVectorFESpace
    fec_type = H1
    fec_order = FIRST
    range_dim = 3
  []
[]

[Variables]
  [displacement]
    type = MFEMVariable
    fespace = H1FESpace
  []
  [velocity]
    type = MFEMVariable
    fespace = H1FESpace
  []
[]

[BCs]
  [fixed_displacement]
    type = MFEMVectorDirichletBC
    variable = displacement
    boundary = '1'
  []
  [fixed_velocity]
    type = MFEMVectorDirichletBC
    variable = velocity
    boundary = '1'
  []
[]

[FunctorMaterials]
  [Beam]
    type = MFEMGenericFunctorMaterial
    prop_names = 'lambda mu density'
    prop_values = '50.0 50.0 1.0'
  []
  [WeightDensity]
    type = MFEMGenericFunctorVectorMaterial
    prop_names = 'gravitational_force_density'
    prop_values = '{0.0 0.0 -1e-2}'
  []
[]

[Kernels]
  # Momentum balance, tested with the displacement test functions.
  [elasticity]
    type = MFEMLinearElasticityKernel
    variable = displacement
    lambda = lambda
    mu = mu
  []
  [inertia]
    type = MFEMTimeDerivativeVectorMassKernel
    variable = displacement
    trial_variable = velocity
    coefficient = density
  []
  [gravity]
    type = MFEMVectorDomainLFKernel
    variable = displacement
    vector_coefficient = gravitational_force_density
  []
  # Kinematic relation v = du/dt, tested with the velocity test functions.
  [displacement_rate]
    type = MFEMTimeDerivativeVectorMassKernel
    variable = velocity
    trial_variable = displacement
  []
  [velocity]
    type = MFEMVectorMassKernel
    variable = velocity
    coefficient = -1.0
  []
[]

[Solvers]
  # The block system coupling displacement and velocity is indefinite.
  [main]
    type = MFEMMUMPS
  []
[]

[Executioner]
  type = MFEMTransient
  device = cpu
  # The first bending period is about 26 time units, so this covers one full swing.
  dt = 0.5
  end_time = 30.0
[]

[Postprocessors]
  [displacement_l2_norm]
    type = MFEMVectorL2Error
    variable = displacement
    function = '0 0 0'
  []
  [velocity_l2_norm]
    type = MFEMVectorL2Error
    variable = velocity
    function = '0 0 0'
  []
[]

[Outputs]
  csv = true
  file_base = OutputData/Elastodynamics
[]
