# Test for the axial stress and strain output for single shell element
# for 2D planar shell with uniform mesh.
# A single shell 1 mm x 1 mm element having Young's Modulus of 5 N/mm^2
# and poissons ratio of 0 is fixed at the left end and
# an axial displacement of 0.2 mm is applied at the right.
# Theoretical value of axial stress and strain are 1 N/mm^2 and 0.2.

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 1
  ny = 1
  xmin = 0.0
  xmax = 1.0
  ymin = 0.0
  ymax = 1.0
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
[]

[AuxKernels]
  [stress_xx]
    type = RankTwoAux
    variable = stress_xx
    rank_two_tensor = global_stress_t_points_1
    index_i = 0
    index_j = 0
  []
  [strain_xx]
    type = RankTwoAux
    variable = strain_xx
    rank_two_tensor = total_global_strain_t_points_1
    index_i = 0
    index_j = 0
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
    variable = disp_x
    boundary = 'right'
    function = displacement
  []
[]

[Functions]
  [displacement]
    type = PiecewiseLinear
    x = '0.0 1.0'
    y = '0.0 0.2'
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
    youngs_modulus = 5.0
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
  [disp_x]
    type = PointValue
    point = '0.5 0.0 0.0'
    variable = disp_z
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
[]

[Outputs]
  exodus = true
[]
