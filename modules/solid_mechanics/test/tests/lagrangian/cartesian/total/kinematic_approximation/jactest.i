# Single trilinear hex element with displacement BCs that produce a known uniform
# F = R * U at every qp, where R = rotation about z by 0.3 rad and
# U = diag(1.1, 1.05, 1.2). F_old = I. Used by the kinematic_approximation
# correctness CSVDiff tests to verify each helper against its closed-form value.
#
# u_i(X) = (F - I)_ij X_j.

theta = 0.3
cos_t = ${fparse cos(theta)}
sin_t = ${fparse sin(theta)}

# Components of F = R*U with U = diag(1.1, 1.05, 1.2).
F11 = ${fparse 1.10 * cos_t}
F12 = ${fparse -1.05 * sin_t}
F21 = ${fparse 1.10 * sin_t}
F22 = ${fparse 1.05 * cos_t}
F33 = 1.20

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  large_kinematics = true
  stabilize_strain = false
[]

[Mesh]
  [msh]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 1
    ny = 1
    nz = 1
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
  [ux]
    type = ParsedFunction
    expression = '${fparse F11 - 1.0} * x + ${F12} * y'
  []
  [uy]
    type = ParsedFunction
    expression = '${F21} * x + ${fparse F22 - 1.0} * y'
  []
  [uz]
    type = ParsedFunction
    expression = '${fparse F33 - 1.0} * z'
  []
[]

[BCs]
  [bx]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 'left right top bottom front back'
    function = ux
    preset = true
  []
  [by]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 'left right top bottom front back'
    function = uy
    preset = true
  []
  [bz]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = 'left right top bottom front back'
    function = uz
    preset = true
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
[]

[Materials]
  [elastic]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1.0
    poissons_ratio = 0.3
  []
  [stress]
    type = ComputeLagrangianLinearElasticStress
  []
  [strain]
    type = ComputeLagrangianStrain
  []
  [dd_11_mat]
    type = RankTwoCartesianComponent
    rank_two_tensor = strain_increment
    property_name = dd_11
    index_i = 0
    index_j = 0
  []
  [dd_22_mat]
    type = RankTwoCartesianComponent
    rank_two_tensor = strain_increment
    property_name = dd_22
    index_i = 1
    index_j = 1
  []
  [dd_33_mat]
    type = RankTwoCartesianComponent
    rank_two_tensor = strain_increment
    property_name = dd_33
    index_i = 2
    index_j = 2
  []
  [dd_12_mat]
    type = RankTwoCartesianComponent
    rank_two_tensor = strain_increment
    property_name = dd_12
    index_i = 0
    index_j = 1
  []
  [dw_12_mat]
    type = RankTwoCartesianComponent
    rank_two_tensor = vorticity_increment
    property_name = dw_12
    index_i = 0
    index_j = 1
  []
[]

[Postprocessors]
  [dd_11]
    type = ElementAverageMaterialProperty
    mat_prop = dd_11
  []
  [dd_22]
    type = ElementAverageMaterialProperty
    mat_prop = dd_22
  []
  [dd_33]
    type = ElementAverageMaterialProperty
    mat_prop = dd_33
  []
  [dd_12]
    type = ElementAverageMaterialProperty
    mat_prop = dd_12
  []
  [dw_12]
    type = ElementAverageMaterialProperty
    mat_prop = dw_12
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  nl_abs_tol = 1e-12
  nl_rel_tol = 1e-10
  dt = 1.0
  num_steps = 1
[]

[Outputs]
  csv = true
[]
