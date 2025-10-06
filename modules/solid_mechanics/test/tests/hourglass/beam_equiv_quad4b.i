[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 50
    ny = 5
    xmax = 1
    ymax = 0.05
    elem_type = QUAD4
    bias_x = 1.05
  []
  [point]
    type = ExtraNodesetGenerator
    input = gen
    new_boundary = fixpoint
    coord = '0 0 0'
  []
[]

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Problem]
  extra_tag_matrices = 'mass'
[]


# [Physics/SolidMechanics/Dynamic]
#   [all]
#     hht_alpha = 0.0
#     mass_damping_coefficient = 0.0
#     stiffness_damping_coefficient = 1.0
#     accelerations = 'accel_x accel_y'
#     generate_output = 'stress_xx stress_yy'
#     strain = FINITE
#     density = density
#     add_variables = true
#     explicit = true
#   []
# []

[Variables]
  [disp_x]
  []
  [disp_y]
  []
[]

[AuxVariables]
  [vel_x]
  []
  [accel_x]
  []
  [vel_y]
  []
  [accel_y]
  []
[]

[AuxKernels]
  [accel_x]
    type = TestNewmarkTI
    variable = accel_x
    displacement = disp_x
    first = false
    execute_on = 'LINEAR TIMESTEP_BEGIN TIMESTEP_END'
  []
  [vel_x]
    type = TestNewmarkTI
    variable = vel_x
    displacement = disp_x
    execute_on = 'LINEAR TIMESTEP_BEGIN TIMESTEP_END'
  []
  [accel_y]
    type = TestNewmarkTI
    variable = accel_y
    displacement = disp_y
    first = false
    execute_on = 'LINEAR TIMESTEP_BEGIN TIMESTEP_END'
  []
  [vel_y]
    type = TestNewmarkTI
    variable = vel_y
    displacement = disp_x
    execute_on = 'LINEAR TIMESTEP_BEGIN TIMESTEP_END'
  []
[]

[Kernels]
  [DynamicTensorMechanics]
    displacements = 'disp_x disp_y'
  []
  [Mass_x]
    type = MassMatrix
    variable = disp_x
    density = density
    matrix_tags = 'mass'
  []
  [Mass_y]
    type = MassMatrix
    variable = disp_y
    density = density
    matrix_tags = 'mass'
  []
  [gravity]
    type = Gravity
    value = -9.81
    variable = disp_y
    function = 'min(t*100,1)'
  []

  # Hourglass correction using the advanced Quad4b kernel
  # Shear modulus mu = E / (2(1 + nu)) = 0.345
  [hourglass_x]
    type = HourglassCorrectionQuad4
    variable = disp_x
    penalty = 1e2
    shear_modulus = 3846154
  []
  [hourglass_y]
    type = HourglassCorrectionQuad4
    variable = disp_y
    penalty = 1e2
    shear_modulus = 3846154
  []
[]

[BCs]
  [left_x]
    type = ExplicitDirichletBC
    boundary = left
    value = 0
    variable = disp_x
  []
  [fix_y]
    type = ExplicitDirichletBC
    boundary = left
    value = 0
    variable = disp_y
  []
[]

[Materials]
  [strain_block]
    type = ComputeFiniteStrain
    displacements = 'disp_x disp_y'
    decomposition_method = HUGHESWINGET
    implicit = false
  []
  [stress]
    type = ComputeFiniteStrainElasticStress
  []
  [stiffness]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 2e7
    poissons_ratio = 0.3
    constant_on = SUBDOMAIN
  []
  [density]
    type = GenericConstantMaterial
    prop_names = density
    prop_values = 1e3
  []
[]

[Executioner]
  type = Transient
  start_time = 0
  num_steps = 40000
  dt = 1e-5
  dtmin = ${Executioner/dt}
  timestep_tolerance = 1e-7

  [TimeIntegrator]
    type = ExplicitMixedOrder
    mass_matrix_tag = 'mass'
    use_constant_mass = true
    second_order_vars = 'disp_x disp_y'
  []
  # [Quadrature]
  #   type = GAUSS
  #   order = CONSTANT
  # []
[]


[Postprocessors]
  [tip_y]
    type = PointValue
    variable = disp_y
    point = '${Mesh/gen/xmax} ${Mesh/gen/ymax} 0'
  []
[]

[Outputs]
  csv = true
  interval = 20
  [exodus]
    type = Exodus
  []
  print_linear_residuals = false
[]

