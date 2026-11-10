# Thin plate on a distorted mesh: the simply supported plate of plate_bending_distributed.i
# with the thickness reduced to 0.05 m (t / a = 1 / 1000) and the load scaled with the
# thickness cubed, q = 10.816 * 0.05^3 = 1.352e-3 N/m^2, so that the thin-plate center
# deflection stays at 2.9971e-3 m. The interior nodes of the 10 x 10 mesh are moved by up
# to 0.3 m with a sine-cosine field, so no element is a rectangle. The four-node element
# has no thin-limit factor: this case checks that it does not lock on a distorted mesh at
# a slenderness where a displacement-based element would be rigid.
[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
    xmin = 0.0
    xmax = 25.0
    ymin = 0.0
    ymax = 25.0
    elem_type = QUAD4
  []
  [distort]
    type = ParsedNodeTransformGenerator
    input = gmg
    x_function = 'x + 0.3 * (x > 0.1) * (x < 24.9) * (y > 0.1) * (y < 24.9) * sin(3 * x) * cos(2 * y)'
    y_function = 'y + 0.3 * (x > 0.1) * (x < 24.9) * (y > 0.1) * (y < 24.9) * cos(2 * x) * sin(3 * y)'
    z_function = 'z'
  []
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
  [rot_z]
    order = FIRST
    family = LAGRANGE
  []
[]

[BCs]
  [symm_left_rot]
    type = DirichletBC
    variable = rot_y
    boundary = left
    value = 0.0
  []
  [symm_bottom_rot]
    type = DirichletBC
    variable = rot_x
    boundary = bottom
    value = 0.0
  []
  [simply_support_x]
    type = DirichletBC
    variable = disp_x
    boundary = 'right top bottom left'
    value = 0.0
  []
  [simply_support_y]
    type = DirichletBC
    variable = disp_y
    boundary = 'right top bottom left'
    value = 0.0
  []
  [simply_support_z]
    type = DirichletBC
    variable = disp_z
    boundary = 'right top'
    value = 0.0
  []
  [hard_support_right]
    type = DirichletBC
    variable = rot_x
    boundary = right
    value = 0.0
  []
  [hard_support_top]
    type = DirichletBC
    variable = rot_y
    boundary = top
    value = 0.0
  []
[]

[Kernels]
  [solid_disp_x]
    type = StressDivergenceDSGShell
    component = 0
    variable = disp_x
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
  []
  [solid_disp_y]
    type = StressDivergenceDSGShell
    component = 1
    variable = disp_y
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
  []
  [solid_disp_z]
    type = StressDivergenceDSGShell
    component = 2
    variable = disp_z
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
  []
  [solid_rot_x]
    type = StressDivergenceDSGShell
    component = 3
    variable = rot_x
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
  []
  [solid_rot_y]
    type = StressDivergenceDSGShell
    component = 4
    variable = rot_y
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
  []
  [solid_rot_z]
    type = StressDivergenceDSGShell
    component = 5
    variable = rot_z
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
  []
  [pressure]
    type = DistributedLoadDSGShell
    variable = disp_z
    displacements = 'disp_x disp_y disp_z'
    function = '1.352e-3'
  []
[]

[Materials]
  [strain]
    type = ComputeDSGShellStrain
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
  []
  [section]
    type = ComputeDSGShellSectionStiffness
    youngs_modulus = 1e9
    poissons_ratio = 0.3
    thickness = 0.05
  []
  [resultants]
    type = ComputeDSGShellResultants
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
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  line_search = 'none'
  # The residual floor of the thin section is round-off of the assembly, about 1e-9 relative.
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-14
  dt = 1.0
  dtmin = 1.0
  end_time = 1.0
[]

[Postprocessors]
  [disp_z_center]
    type = PointValue
    point = '0.0 0.0 0.0'
    variable = disp_z
  []
[]

[Outputs]
  csv = true
[]
