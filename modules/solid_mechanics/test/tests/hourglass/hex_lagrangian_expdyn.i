# Regular-MOOSE reduced-integration explicit dynamics on HEX8: total-Lagrangian
# kernels + St. Venant-Kirchhoff stress at a single quadrature point, stabilized
# by HourglassCorrectionHex8. A 1x1x4 column is pulled axially.
[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  large_kinematics = true
[]

[Problem]
  extra_tag_matrices = 'mass'
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 1
    ny = 1
    nz = 4
    zmax = 4
    elem_type = HEX8
  []
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
  [disp_z]
  []
[]

[Functions]
  [forcing_fn]
    type = PiecewiseLinear
    x = '0.0 0.1 0.2    0.3  0.4    0.5  0.6'
    y = '0.0 0.0 0.0025 0.01 0.0175 0.02 0.02'
  []
[]

[Kernels]
  [sdx]
    type = TotalLagrangianStressDivergence
    variable = disp_x
    component = 0
  []
  [sdy]
    type = TotalLagrangianStressDivergence
    variable = disp_y
    component = 1
  []
  [sdz]
    type = TotalLagrangianStressDivergence
    variable = disp_z
    component = 2
  []
  [hgx]
    type = HourglassCorrectionHex8
    variable = disp_x
    penalty = 0.05
    shear_modulus = 0.385
  []
  [hgy]
    type = HourglassCorrectionHex8
    variable = disp_y
    penalty = 0.05
    shear_modulus = 0.385
  []
  [hgz]
    type = HourglassCorrectionHex8
    variable = disp_z
    penalty = 0.05
    shear_modulus = 0.385
  []
  [mx]
    type = MassMatrix
    density = density
    variable = disp_x
    matrix_tags = 'mass'
  []
  [my]
    type = MassMatrix
    density = density
    variable = disp_y
    matrix_tags = 'mass'
  []
  [mz]
    type = MassMatrix
    density = density
    variable = disp_z
    matrix_tags = 'mass'
  []
[]

[Materials]
  [density]
    type = GenericConstantMaterial
    prop_names = 'density'
    prop_values = 1
  []
  [C]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1
    poissons_ratio = 0.3
  []
  [strain]
    type = ComputeLagrangianStrain
  []
  [stress]
    type = ComputeStVenantKirchhoffStress
  []
[]

[BCs]
  [pull_z]
    type = ExplicitFunctionDirichletBC
    variable = disp_z
    boundary = 'front'
    function = forcing_fn
  []
  [fix_z]
    type = DirichletBC
    variable = disp_z
    boundary = 'back'
    value = 0
  []
  [fix_x]
    type = DirichletBC
    variable = disp_x
    boundary = 'left'
    value = 0
  []
  [fix_y]
    type = DirichletBC
    variable = disp_y
    boundary = 'bottom'
    value = 0
  []
[]

[Postprocessors]
  [disp_z]
    type = ElementAverageValue
    variable = disp_z
    execute_on = 'TIMESTEP_END'
  []
[]

[Executioner]
  type = Transient
  [TimeIntegrator]
    type = ExplicitMixedOrder
    mass_matrix_tag = 'mass'
    use_constant_mass = true
    second_order_vars = 'disp_x disp_y disp_z'
  []
  [Quadrature]
    type = GAUSS
    order = CONSTANT
  []
  start_time = 0.0
  num_steps = 30
  dt = 0.02
[]

[Outputs]
  csv = true
[]
