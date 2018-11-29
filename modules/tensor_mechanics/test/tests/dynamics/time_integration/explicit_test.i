# Test for Explicit Euler integration

# The test is for an 1D bar element of  unit length fixed on one end
# with a ramped pressure boundary condition applied to the other end.
# The equation of motion in terms of matrices is:
#
# M*accel + K*disp = P*Area
#
# Here M is the mass matrix, K is the stiffness matrix, P is the applied pressure
#
# This equation is equivalent to:
#
# density*accel + Div Stress = P
#
# The first term on the left is evaluated using the Inertial force kernel
# The last term on the left is evaluated using StressDivergenceTensors
# The residual due to Pressure is evaluated using Pressure boundary condition

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1
  ny = 1
  nz = 1
  xmin = 0.0
  xmax = 0.1
  ymin = 0.0
  ymax = 1.0
  zmin = 0.0
  zmax = 0.1
[]


[Variables]
  [disp_x]
  []
  [disp_y]
  []
  [disp_z]
  []
[]

[AuxVariables]
  [stress_yy]
    order = CONSTANT
    family = MONOMIAL
  []
  [strain_yy]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[Kernels]
  [TensorMechanics]
    displacements = 'disp_x disp_y disp_z'
  []
  [inertia_x]
    type = InertialForceExplicit
    variable = disp_x
  []
  [inertia_y]
    type = InertialForceExplicit
    variable = disp_y
  []
  [inertia_z]
    type = InertialForceExplicit
    variable = disp_z
  []
[]

[AuxKernels]
  [stress_yy]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_yy
    index_i = 0
    index_j = 1
  []
  [strain_yy]
    type = RankTwoAux
    rank_two_tensor = total_strain
    variable = strain_yy
    index_i = 0
    index_j = 1
  []
[]

[BCs]
  [top_y]
    type = DirichletBC
    variable = disp_y
    boundary = top
    value = 0.0
    extra_matrix_tags = 'secondtime'
  []
  [top_x]
    type = DirichletBC
    variable = disp_x
    boundary = top
    value = 0.0
    extra_matrix_tags = 'secondtime'
  []
  [top_z]
    type = DirichletBC
    variable = disp_z
    boundary = top
    value = 0.0
    extra_matrix_tags = 'secondtime'
  []
  [bottom_x]
    type = DirichletBC
    variable = disp_x
    boundary = bottom
    value = 0.0
    extra_matrix_tags = 'secondtime'
  []
  [bottom_z]
    type = DirichletBC
    variable = disp_z
    boundary = bottom
    value = 0.0
    extra_matrix_tags = 'secondtime'
  []
  [Pressure]
    [Side1]
      boundary = bottom
      function = pressure
      disp_x = disp_x
      disp_y = disp_y
      disp_z = disp_z
      factor = 1
    []
  []
[]

[Materials]
  [Elasticity_tensor]
    type = ComputeElasticityTensor
    fill_method = symmetric_isotropic
    C_ijkl = '210e9 0'
  []
  [strain]
    type = ComputeSmallStrain
    displacements = 'disp_x disp_y disp_z'
  []
  [stress]
    type = ComputeLinearElasticStress
  []
  [density]
    type = GenericConstantMaterial
    prop_names = 'density'
    prop_values = '7750'
  []
[]

[Executioner]
  type = Transient
  start_time = 0
  end_time = 2
  dt = 2e-4
  [TimeIntegrator]
    type = ActuallyExplicitEuler
    solve_type = lumped
  []
[]

[Functions]
  [pressure]
    type = PiecewiseLinear
    x = '0.0 0.1 0.2 1.0 2.0 5.0'
    y = '0.0 0.1 0.2 1.0 1.0 1.0'
    scale_factor = 1e9
  []
[]

[Postprocessors]
  [_dt]
    type = TimestepSize
  []
  [disp]
    type = NodalMaxValue
    variable = disp_y
    boundary = bottom
  []
  [stress_yy]
    type = ElementAverageValue
    variable = stress_yy
  []
  [strain_yy]
    type = ElementAverageValue
    variable = strain_yy
  []
[]

[Outputs]
  interval = 500
  exodus = true
  perf_graph = true
[]
