# Test for Rayleigh mass damping implemented using ExplicitMixedOrder time integration

# The test is for an 1D bar element of unit length fixed on one end
# with a ramped pressure boundary condition applied to the other end.
# The equation of motion in terms of matrices is:
#
# M*accel + eta*M*vel + zeta*K*vel + K*disp = P*Area
#
# Here M is the mass matrix, K is the stiffness matrix, P is the applied pressure.
# zeta and eta correspond to the stiffness and mass proportional Rayleigh damping,
# and in this test zeta = 0
#

zeta = 0
eta = 1
young = 8
poisson = 0
density = 2
pressure = -1
dt = 0.1

############
# By-hand calculation of solution
# The single cubical element of side length 2 has
#   area of node = 1, so P*area = -1
#   K * disp = force on node due to disp = stress*area = (young*disp/2)*1 = 4*disp
#   M = nodal mass = density * 2^3 / 8 = 2
# time step 1:
#   residual = P*area - K*disp - eta*M*vel = -1
#   accel = residual/mass = -0.5
#   vel += accel * dt = -0.05
#   disp += vel * dt = -0.005
# time step 2:
#   residual = P*area - K*disp - eta*M*vel = -1 - 4*(-0.005) - 1*2*(-0.05)= -0.88
#   accel = residual/mass = -0.44
#   vel += accel * dt = -0.094
#   disp += vel * dt = -0.0144

############
# Setting implicit = false in an object means:
#  - Jacobian terms will not be calculated
#  - old values will be used (for instance, old displacement,
#    not displacement that is being calculated at current time step)
# So, implicit = false is the standard in most objects in ExplicitMixedOrder
# Exceptions:
#  - MassMatrix (because MOOSE treats the mass matrix as a Jacobian)
#  - AuxKernels and Postprocessors (want to use most recent variables)
# It is irrelevant for some things, such as ComputeIsotropicElasticityTensor

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1
  ny = 1
  nz = 1
  xmin = 0
  xmax = 2
  ymin = 0
  ymax = 2
  zmin = 0
  zmax = 2
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Problem]
  extra_tag_matrices = 'mass'
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
  [disp_z]
  []
[]

[Kernels]
  [TM_DynamicSolidMechanics0]
    type = DynamicStressDivergenceTensors
    alpha = 0
    component = 0
    use_displaced_mesh = false
    variable = disp_x
    zeta = ${zeta}
    implicit = false
  []
  [TM_DynamicSolidMechanics1]
    type = DynamicStressDivergenceTensors
    alpha = 0
    component = 1
    use_displaced_mesh = false
    variable = disp_y
    zeta = ${zeta}
    implicit = false
  []
  [TM_DynamicSolidMechanics2]
    type = DynamicStressDivergenceTensors
    alpha = 0
    component = 2
    use_displaced_mesh = false
    variable = disp_z
    zeta = ${zeta}
    implicit = false
  []
  [massmatrix_x]
    type = MassMatrix
    density = density
    matrix_tags = 'mass'
    variable = disp_x
  []
  [massmatrix_y]
    type = MassMatrix
    density = density
    matrix_tags = 'mass'
    variable = disp_y
  []
  [massmatrix_z]
    type = MassMatrix
    density = density
    matrix_tags = 'mass'
    variable = disp_z
  []
[]

[NodalKernels]
  [damping_x]
    type = ExplicitMassDamping
    variable = disp_x
    eta = ${eta}
    implicit = false
  []
  [damping_y]
    type = ExplicitMassDamping
    variable = disp_y
    eta = ${eta}
    implicit = false
  []
  [damping_z]
    type = ExplicitMassDamping
    variable = disp_z
    eta = ${eta}
    implicit = false
  []
[]

[BCs]
  [Pressure]
    [Side1]
      boundary = bottom
      postprocessor = ${pressure}
      displacements = 'disp_x disp_y disp_z'
    []
  []
  [top_y]
    type = ExplicitDirichletBC
    variable = disp_y
    boundary = top
    value = 0.0
    implicit = false
  []
  [zero_x]
    type = ExplicitDirichletBC
    variable = disp_x
    boundary = 'top bottom'
    value = 0.0
    implicit = false
  []
  [zero_z]
    type = ExplicitDirichletBC
    variable = disp_z
    boundary = 'top bottom'
    value = 0.0
    implicit = false
  []
[]

[Materials]
  [Elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = ${young}
    poissons_ratio = ${poisson}
    implicit = false
  []

  [strain]
    type = ComputeSmallStrain
    implicit = false
  []

  [stress]
    type = ComputeLinearElasticStress
    implicit = false
  []

  [density]
    type = GenericConstantMaterial
    prop_names = density
    prop_values = ${density}
    implicit = false
  []
[]

[Executioner]
  type = Transient

  [TimeIntegrator]
    type = ExplicitMixedOrder
    mass_matrix_tag = 'mass'
    second_order_vars = 'disp_x disp_y disp_z'
    use_constant_mass = true
  []

  start_time = 0
  end_time = 0.2
  dt = ${dt}
[]

[Postprocessors]
  [disp]
    type = NodalExtremeValue
    variable = disp_y
    boundary = bottom
  []
[]

[Outputs]
  csv = true
[]
