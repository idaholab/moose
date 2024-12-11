# Static test for the inclined shell element.
# A single shell element is oriented at a 45 deg. angle with respect to the Y axis.
# One end of the shell is fixed and an axial deformation to the shell element is
# applied at the other end by resolving the deformation into Y and Z direction.
# The local stresses are computed and stored in aux variables.
# The local stress_22 should be zero (because of plane stress condition).

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
  [stress_00]
    order = CONSTANT
    family = MONOMIAL
  []
  [stress_11]
    order = CONSTANT
    family = MONOMIAL
  []
  [stress_22]
    order = CONSTANT
    family = MONOMIAL
  []
  [stress_01]
    order = CONSTANT
    family = MONOMIAL
  []
  [stress_02]
    order = CONSTANT
    family = MONOMIAL
  []
  [stress_12]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [stress_00]
    type = RankTwoAux
    variable = stress_00
    rank_two_tensor = local_stress_t_points_0
    index_i = 0
    index_j = 0
    execute_on = TIMESTEP_END
  []
  [stress_11]
    type = RankTwoAux
    variable = stress_11
    rank_two_tensor = local_stress_t_points_0
    index_i = 1
    index_j = 1
    execute_on = TIMESTEP_END
  []
  [stress_22]
    type = RankTwoAux
    variable = stress_22
    rank_two_tensor = local_stress_t_points_0
    index_i = 2
    index_j = 2
    execute_on = TIMESTEP_END
  []
  [stress_01]
    type = RankTwoAux
    variable = stress_01
    rank_two_tensor = local_stress_t_points_0
    index_i = 0
    index_j = 1
    execute_on = TIMESTEP_END
  []
  [stress_02]
    type = RankTwoAux
    variable = stress_02
    rank_two_tensor = local_stress_t_points_0
    index_i = 0
    index_j = 2
    execute_on = TIMESTEP_END
  []
  [stress_12]
    type = RankTwoAux
    variable = stress_12
    rank_two_tensor = local_stress_t_points_0
    index_i = 1
    index_j = 2
    execute_on = TIMESTEP_END
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
  [stress_11_el_0]
    type = ElementalVariableValue
    elementid = 0
    variable = stress_11
  []

  [stress_12_el_0]
    type = ElementalVariableValue
    elementid = 0
    variable = stress_12
  []

  [stress_00_el_0]
    type = ElementalVariableValue
    elementid = 0
    variable = stress_00
  []

  [stress_01_el_0]
    type = ElementalVariableValue
    elementid = 0
    variable = stress_01
  []

  [stress_02_el_0]
    type = ElementalVariableValue
    elementid = 0
    variable = stress_02
  []

  [stress_22_el_0]
    type = ElementalVariableValue
    elementid = 0
    variable = stress_22
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
