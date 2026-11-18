# Simply supported plate strip under two concentrated line loads, DSG shell
# elements.
#
# A plate of 9 m x 1 m in the XZ plane, E = 200e9, nu = 0 and thickness
# 0.133887 m, is simply supported at x = 0 and x = 9 and loaded by two equal
# line loads F = 10000 N/m at x = 3 and x = 6, applied as -2500 N on each of
# the four nodes of those lines. The mesh file is the one of the module's
# shell battery.
#
# Beam theory gives the maximum deflection at the center 6.469e-3 m, the
# bending moment at the center 30000 N m per unit width, and the transverse
# shear force 10000 for 0 < x < 3, 0 for 3 < x < 6 and -10000 for 6 < x < 9.

[Mesh]
  [gmg]
    type = FileMeshGenerator
    file = plate_concentrated_loads.msh
  []
  [p1]
    type = BoundingBoxNodeSetGenerator
    input = gmg
    bottom_left = '2.99 0.0 -0.1'
    top_right = '3.1 0.0 1.1'
    new_boundary = 100
  []
  [p2]
    type = BoundingBoxNodeSetGenerator
    input = p1
    bottom_left = '5.99 0.0 -0.1'
    top_right = '6.1 0.0 1.1'
    new_boundary = 101
  []
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
  [rot_z]
  []
[]

[AuxVariables]
  [moment_11]
    order = CONSTANT
    family = MONOMIAL
  []
  [shear_1]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [moment_11]
    type = DSGShellResultantsAux
    variable = moment_11
    resultant = bending_moment_11
    execute_on = TIMESTEP_END
  []
  [shear_1]
    type = DSGShellResultantsAux
    variable = shear_1
    resultant = shear_force_1
    execute_on = TIMESTEP_END
  []
[]

[BCs]
  [simply_support_x]
    type = DirichletBC
    variable = disp_x
    boundary = 'right left'
    value = 0.0
  []
  [simply_support_y]
    type = DirichletBC
    variable = disp_y
    boundary = 'right left'
    value = 0.0
  []
[]

[NodalKernels]
  [force_p1]
    type = ConstantRate
    variable = disp_y
    boundary = 100
    rate = -2500
  []
  [force_p2]
    type = ConstantRate
    variable = disp_y
    boundary = 101
    rate = -2500
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
[]

[Materials]
  [strain]
    type = ComputeDSGShellStrain
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
  []
  [section]
    type = ComputeDSGShellSectionStiffness
    youngs_modulus = 200e9
    poissons_ratio = 0.0
    thickness = 0.133887
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
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-6
  dt = 1.0
  dtmin = 1.0
  end_time = 1.0
[]

[Postprocessors]
  [disp_y_center]
    type = PointValue
    point = '4.5 0.0 0.5'
    variable = disp_y
  []
  [moment_11_center]
    type = PointValue
    point = '4.5 0.0 0.5'
    variable = moment_11
  []
  [shear_1_first_span]
    type = PointValue
    point = '1.5 0.0 0.5'
    variable = shear_1
  []
  [shear_1_middle_span]
    type = PointValue
    point = '4.5 0.0 0.5'
    variable = shear_1
  []
  [shear_1_last_span]
    type = PointValue
    point = '7.5 0.0 0.5'
    variable = shear_1
  []
[]

[Outputs]
  csv = true
[]
