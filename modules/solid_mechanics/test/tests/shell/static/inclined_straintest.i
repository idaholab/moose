# Static test for the inclined shell element.
# A single shell element is oriented at a 45 deg. angle with respect to the Y axis.
# One end of the shell is fixed and an axial deformation to the shell element is
# applied at the other end by resolving the deformation into Y and Z direction.
# The stress and strain result in the global orientation when transformed to
# the shell oriention gives the correct value of the axial stress and strain.

[Mesh]
  type = FileMesh
  file = shell_inclined.e
  displacements = 'disp_x disp_y disp_z'
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
  [disp_z]
  []
  [rot_x]
  []
  [rot_y]
  []
[]

[AuxVariables]
  [stress_xx]
    order = CONSTANT
    family = MONOMIAL
  []
  [strain_xx]
    order = CONSTANT
    family = MONOMIAL
  []
  [stress_yy]
    order = CONSTANT
    family = MONOMIAL
  []
  [strain_yy]
    order = CONSTANT
    family = MONOMIAL
  []
  [stress_xy]
    order = CONSTANT
    family = MONOMIAL
  []
  [strain_xy]
    order = CONSTANT
    family = MONOMIAL
  []
  [stress_yz]
    order = CONSTANT
    family = MONOMIAL
  []
  [strain_yz]
    order = CONSTANT
    family = MONOMIAL
  []
  [stress_xz]
    order = CONSTANT
    family = MONOMIAL
  []
  [strain_xz]
    order = CONSTANT
    family = MONOMIAL
  []
  [stress_zz]
    order = CONSTANT
    family = MONOMIAL
  []
  [strain_zz]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [stress_xx]
    type = RankTwoAux
    variable = stress_xx
    selected_qp = 0
    rank_two_tensor = global_stress_t_points_0
    index_i = 0
    index_j = 0
  []
  [strain_xx]
    type = RankTwoAux
    variable = strain_xx
    rank_two_tensor = total_global_strain_t_points_0
    selected_qp = 0
    index_i = 0
    index_j = 0
  []
  [stress_yy]
    type = RankTwoAux
    variable = stress_yy
    rank_two_tensor = global_stress_t_points_0
    selected_qp = 0
    index_i = 1
    index_j = 1
  []
  [strain_yy]
    type = RankTwoAux
    variable = strain_yy
    rank_two_tensor = total_global_strain_t_points_0
    selected_qp = 0
    index_i = 1
    index_j = 1
  []
  [stress_xy]
    type = RankTwoAux
    variable = stress_xy
    rank_two_tensor = global_stress_t_points_0
    selected_qp = 0
    index_i = 0
    index_j = 1
  []
  [strain_xy]
    type = RankTwoAux
    variable = strain_xy
    rank_two_tensor = total_global_strain_t_points_0
    selected_qp = 0
    index_i = 0
    index_j = 1
  []
  [stress_yz]
    type = RankTwoAux
    variable = stress_yz
    rank_two_tensor = global_stress_t_points_0
    selected_qp = 0
    index_i = 1
    index_j = 2
  []
  [strain_yz]
    type = RankTwoAux
    variable = strain_yz
    rank_two_tensor = total_global_strain_t_points_0
    selected_qp = 0
    index_i = 1
    index_j = 2
  []
  [stress_xz]
    type = RankTwoAux
    variable = stress_xz
    rank_two_tensor = global_stress_t_points_0
    selected_qp = 0
    index_i = 0
    index_j = 2
  []
  [strain_xz]
    type = RankTwoAux
    variable = strain_xz
    rank_two_tensor = total_global_strain_t_points_0
    selected_qp = 0
    index_i = 0
    index_j = 2
  []
  [stress_zz]
    type = RankTwoAux
    variable = stress_zz
    rank_two_tensor = global_stress_t_points_0
    selected_qp = 0
    index_i = 2
    index_j = 2
  []
  [strain_zz]
    type = RankTwoAux
    variable = strain_zz
    rank_two_tensor = total_global_strain_t_points_0
    selected_qp = 0
    index_i = 2
    index_j = 2
  []
[]

[BCs]
  [fixy1]
    type = DirichletBC
    variable = disp_y
    boundary = '0'
    value = 0.0
  []
  [fixz1]
    type = DirichletBC
    variable = disp_z
    boundary = '0'
    value = 0.0
  []
  [fixr1]
    type = DirichletBC
    variable = rot_x
    boundary = '0'
    value = 0.0
  []
  [fixr2]
    type = DirichletBC
    variable = rot_y
    boundary = '0'
    value = 0.0
  []
  [fixx1]
    type = DirichletBC
    variable = disp_x
    boundary = '0'
    value = 0.0
  []
  [dispz]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = '2'
    function = force_function
  []
  [dispy]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = '2'
    function = force_function
  []
[]

[Functions]
  [force_function]
    type = PiecewiseLinear
    x = '0.0 1'
    y = '0.0 0.33535534'
  []
[]

[Kernels]
  [solid_disp_x]
    type = ADStressDivergenceShell
    block = '0'
    component = 0
    variable = disp_x
    through_thickness_order = SECOND
  []
  [solid_disp_y]
    type = ADStressDivergenceShell
    block = '0'
    component = 1
    variable = disp_y
    through_thickness_order = SECOND
  []
  [solid_disp_z]
    type = ADStressDivergenceShell
    block = '0'
    component = 2
    variable = disp_z
    through_thickness_order = SECOND
  []
  [solid_rot_x]
    type = ADStressDivergenceShell
    block = '0'
    component = 3
    variable = rot_x
    through_thickness_order = SECOND
  []
  [solid_rot_y]
    type = ADStressDivergenceShell
    block = '0'
    component = 4
    variable = rot_y
    through_thickness_order = SECOND
  []
[]

[Materials]
  [elasticity]
    type = ADComputeIsotropicElasticityTensorShell
    youngs_modulus = 5
    poissons_ratio = 0.0
    block = 0
    through_thickness_order = SECOND
  []
  [strain]
    type = ADComputeIncrementalShellStrain
    block = '0'
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y'
    thickness = 0.1
    through_thickness_order = SECOND
  []
  [stress]
    type = ADComputeShellStress
    block = 0
    through_thickness_order = SECOND
  []
[]

[Postprocessors]
  [stress_yy_el_0]
    type = ElementalVariableValue
    elementid = 0
    variable = stress_yy
  []
  [strain_yy_el_0]
    type = ElementalVariableValue
    elementid = 0
    variable = strain_yy
  []
  [stress_yz_el_0]
    type = ElementalVariableValue
    elementid = 0
    variable = stress_yz
  []
  [strain_yz_el_0]
    type = ElementalVariableValue
    elementid = 0
    variable = strain_yz
  []
  [stress_xx_el_0]
    type = ElementalVariableValue
    elementid = 0
    variable = stress_xx
  []
  [strain_xx_el_0]
    type = ElementalVariableValue
    elementid = 0
    variable = strain_xx
  []
  [stress_xy_el_0]
    type = ElementalVariableValue
    elementid = 0
    variable = stress_xy
  []
  [strain_xy_el_0]
    type = ElementalVariableValue
    elementid = 0
    variable = strain_xy
  []
  [stress_xz_el_0]
    type = ElementalVariableValue
    elementid = 0
    variable = stress_xz
  []
  [strain_xz_el_0]
    type = ElementalVariableValue
    elementid = 0
    variable = strain_xz
  []
  [stress_zz_el_0]
    type = ElementalVariableValue
    elementid = 0
    variable = stress_zz
  []
  [strain_zz_el_0]
    type = ElementalVariableValue
    elementid = 0
    variable = strain_zz
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  solve_type = PJFNK
  l_tol = 1e-11
  nl_max_its = 15
  nl_rel_tol = 1e-11
  nl_abs_tol = 1e-10
  l_max_its = 20
  dt = 1
  dtmin = 0.01
  timestep_tolerance = 2e-13
  end_time = 1
[]

[Outputs]
  exodus = true
[]
