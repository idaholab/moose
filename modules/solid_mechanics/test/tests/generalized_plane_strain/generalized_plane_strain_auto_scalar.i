[Mesh]
  [square]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
  []
  displacements = 'disp_x disp_y'
[]

[Problem]
  nl_sys_names = 'unused mechanics'
[]

[Variables]
  [unused]
    solver_sys = unused
    outputs = none
  []
  [disp_x]
    solver_sys = mechanics
  []
  [disp_y]
    solver_sys = mechanics
  []
[]

[AuxVariables]
  [saved_x]
    order = FIRST
    family = LAGRANGE
  []
  [saved_y]
    order = FIRST
    family = LAGRANGE
  []

  [stress_xx]
    order = CONSTANT
    family = MONOMIAL
  []
  [stress_xy]
    order = CONSTANT
    family = MONOMIAL
  []
  [stress_yy]
    order = CONSTANT
    family = MONOMIAL
  []
  [stress_zz]
    order = CONSTANT
    family = MONOMIAL
  []

  [strain_xx]
    order = CONSTANT
    family = MONOMIAL
  []
  [strain_xy]
    order = CONSTANT
    family = MONOMIAL
  []
  [strain_yy]
    order = CONSTANT
    family = MONOMIAL
  []
  [strain_zz]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[Postprocessors]
  [react_z]
    type = ADMaterialTensorIntegral
    rank_two_tensor = stress
    index_i = 2
    index_j = 2
  []
[]

[Physics]
  [SolidMechanics]
    [GeneralizedPlaneStrain]
      [gps]
        use_automatic_differentiation = true
        use_displaced_mesh = true
        displacements = 'disp_x disp_y'
        scalar_out_of_plane_strain = scalar_strain_zz
        out_of_plane_pressure_function = traction_function
        pressure_factor = 1e5
      []
    []
  []
[]

[Kernels]
  [unused]
    type = ADReaction
    variable = unused
  []
  [SolidMechanics]
    use_automatic_differentiation = true
    use_displaced_mesh = false
    displacements = 'disp_x disp_y'
    save_in = 'saved_x saved_y'
  []
[]

[AuxKernels]
  [stress_xx]
    type = ADRankTwoAux
    rank_two_tensor = stress
    variable = stress_xx
    index_i = 0
    index_j = 0
  []
  [stress_xy]
    type = ADRankTwoAux
    rank_two_tensor = stress
    variable = stress_xy
    index_i = 0
    index_j = 1
  []
  [stress_yy]
    type = ADRankTwoAux
    rank_two_tensor = stress
    variable = stress_yy
    index_i = 1
    index_j = 1
  []
  [stress_zz]
    type = ADRankTwoAux
    rank_two_tensor = stress
    variable = stress_zz
    index_i = 2
    index_j = 2
  []

  [strain_xx]
    type = ADRankTwoAux
    rank_two_tensor = total_strain
    variable = strain_xx
    index_i = 0
    index_j = 0
  []
  [strain_xy]
    type = ADRankTwoAux
    rank_two_tensor = total_strain
    variable = strain_xy
    index_i = 0
    index_j = 1
  []
  [strain_yy]
    type = ADRankTwoAux
    rank_two_tensor = total_strain
    variable = strain_yy
    index_i = 1
    index_j = 1
  []
  [strain_zz]
    type = ADRankTwoAux
    rank_two_tensor = total_strain
    variable = strain_zz
    index_i = 2
    index_j = 2
  []
[]

[Functions]
  [traction_function]
    type = PiecewiseLinear
    x = '0  2'
    y = '0  1'
  []
[]

[BCs]
  [leftx]
    type = DirichletBC
    boundary = 3
    variable = disp_x
    value = 0.0
  []
  [bottomy]
    type = DirichletBC
    boundary = 0
    variable = disp_y
    value = 0.0
  []
[]

[Materials]
  [elastic_tensor]
    type = ADComputeIsotropicElasticityTensor
    poissons_ratio = 0.3
    youngs_modulus = 1e6
  []
  [strain]
    type = ADComputePlaneSmallStrain
    displacements = 'disp_x disp_y'
    scalar_out_of_plane_strain = scalar_strain_zz
  []
  [stress]
    type = ADComputeLinearElasticStress
  []
  [traction_material]
    type = ADGenericFunctionMaterial
    prop_names = traction_material
    prop_values = traction_function
  []
[]

[Executioner]
  type = Transient

  solve_type = PJFNK
  line_search = none

  l_max_its = 100
  l_tol = 1e-4

  nl_max_its = 15
  nl_rel_tol = 1e-14
  nl_abs_tol = 1e-11

  start_time = 0.0
  dt = 1.0
  dtmin = 1.0
  end_time = 2.0
  num_steps = 2
[]

[Outputs]
  exodus = true
  file_base = out_of_plane_pressure_out
[]
