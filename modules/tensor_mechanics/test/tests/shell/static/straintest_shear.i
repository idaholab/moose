# Test for the shear stress and strain output for 2D planar shell with uniform mesh.
# A  cantiliver beam of length 10 m  and cross-section 1.5 m x 0.1 m having
# Young's Modulus of 5 N/mm^2 and poissons ratio of 0 is subjected to shear
# displacement of 0.05 m at the free end.

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 4
  ny = 1
  xmin = 0.0
  xmax = 10
  ymin = 0.0
  ymax = 1.5
[]

[Variables]
  [disp_x]
    order = FIRST
    family = LAGRANGE
  []
  [disp_y]
    order = FIRST
    family = LAGRANGE
  []
  [disp_z]
    order = FIRST
    family = LAGRANGE
  []
  [rot_x]
    order = FIRST
    family = LAGRANGE
  []
  [rot_y]
    order = FIRST
    family = LAGRANGE
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
[]

[AuxKernels]
  [stress_xx]
    type = RankTwoAux
    variable = stress_xx
    selected_qp = 0
    rank_two_tensor = global_stress_t_points_1
    index_i = 0
    index_j = 0
  []
  [strain_xx]
    type = RankTwoAux
    variable = strain_xx
    rank_two_tensor = total_global_strain_t_points_1
    selected_qp = 0
    index_i = 0
    index_j = 0
  []
  [stress_yy]
    type = RankTwoAux
    variable = stress_yy
    rank_two_tensor = global_stress_t_points_1
    selected_qp = 0
    index_i = 1
    index_j = 1
  []
  [strain_yy]
    type = RankTwoAux
    variable = strain_yy
    rank_two_tensor = total_global_strain_t_points_1
    selected_qp = 0
    index_i = 1
    index_j = 1
  []
  [stress_xy]
    type = RankTwoAux
    variable = stress_xy
    rank_two_tensor = global_stress_t_points_1
    selected_qp = 0
    index_i = 0
    index_j = 1
  []
  [strain_xy]
    type = RankTwoAux
    variable = strain_xy
    rank_two_tensor = total_global_strain_t_points_1
    selected_qp = 0
    index_i = 0
    index_j = 1
  []
  [stress_yz]
    type = RankTwoAux
    variable = stress_yz
    rank_two_tensor = global_stress_t_points_1
    selected_qp = 0
    index_i = 1
    index_j = 2
  []
  [strain_yz]
    type = RankTwoAux
    variable = strain_yz
    rank_two_tensor = total_global_strain_t_points_1
    selected_qp = 0
    index_i = 1
    index_j = 2
  []
  [stress_xz]
    type = RankTwoAux
    variable = stress_xz
    rank_two_tensor = global_stress_t_points_1
    selected_qp = 0
    index_i = 0
    index_j = 2
  []
  [strain_xz]
    type = RankTwoAux
    variable = strain_yz
    rank_two_tensor = total_global_strain_t_points_1
    selected_qp = 0
    index_i = 0
    index_j = 2
  []
[]

[BCs]
  [fixx]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0.0
  []
  [fixy]
    type = DirichletBC
    variable = disp_y
    boundary = left
    value = 0.0
  []
  [fixz]
    type = DirichletBC
    variable = disp_z
    boundary = left
    value = 0.0
  []
  [fixr1]
    type = DirichletBC
    variable = rot_x
    boundary = left
    value = 0.0
  []
  [fixr2]
    type = DirichletBC
    variable = rot_y
    boundary = left
    value = 0.0
  []
  [disp]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 'right'
    function = displacement
  []
[]

[Functions]
  [displacement]
    type = PiecewiseLinear
    x = '0.0 1.0'
    y = '0.0 0.05'
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
  solve_type = NEWTON
  automatic_scaling = true

  line_search = 'none'
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-14

  dt = 1
  dtmin = 1
  end_time = 1
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
    youngs_modulus = 4.0e6
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
  [stress_xy_el_1]
    type = ElementalVariableValue
    elementid = 1
    variable = stress_xy
  []
  [strain_xy_el_1]
    type = ElementalVariableValue
    elementid = 1
    variable = strain_xy
  []
  [stress_xy_el_2]
    type = ElementalVariableValue
    elementid = 2
    variable = stress_xy
  []
  [strain_xy_el_2]
    type = ElementalVariableValue
    elementid = 2
    variable = strain_xy
  []
  [stress_xy_el_3]
    type = ElementalVariableValue
    elementid = 3
    variable = stress_xy
  []
  [strain_xy_el_3]
    type = ElementalVariableValue
    elementid = 3
    variable = strain_xy
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
  [stress_xx_el_1]
    type = ElementalVariableValue
    elementid = 1
    variable = stress_xx
  []
  [strain_xx_el_1]
    type = ElementalVariableValue
    elementid = 1
    variable = strain_xx
  []
  [stress_xx_el_2]
    type = ElementalVariableValue
    elementid = 2
    variable = stress_xx
  []
  [strain_xx_el_2]
    type = ElementalVariableValue
    elementid = 2
    variable = strain_xx
  []
  [stress_xx_el_3]
    type = ElementalVariableValue
    elementid = 3
    variable = stress_xx
  []
  [strain_xx_el_3]
    type = ElementalVariableValue
    elementid = 3
    variable = strain_xx
  []
[]

[Outputs]
  exodus = true
[]
