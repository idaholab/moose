# Bending moment of the cantilever strip of cantilever_strip.i recovered at the nodes.
#
# The strip of 10 m by 1 m, thickness 0.1, E = 2100000 and nu = 0, clamped at the
# bottom edge and loaded by P = 1 N at the top edge, on four elements along its length.
# The bending moment is P (L - y) in magnitude: 10 at the clamp, 7.5, 5 and 2.5 at the nodes
# between the elements, 0 at the tip; it is reported negative, the loaded face being in
# compression (the moment convention of ShellResultantsAux).
#
# Every element evaluates its resultants at its own nodes from its own interpolation,
# and the nodal value is the average over the elements sharing the node. On the
# four-node element the curvature is constant along the strip, so each element reports
# its center moment at all of its nodes: the average at a node between two elements is
# the exact linear moment there (7.5, 5, 2.5), and the clamp and the tip, which belong
# to one element each, report that element's center moment (8.75 and 1.25). On the
# eight-node element the curvature is linear within the element and every nodal moment
# is exact, the clamp included.

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 1
    ny = 4
    xmin = 0.0
    xmax = 1.0
    ymin = 0.0
    ymax = 10.0
    elem_type = QUAD4
  []
  [tip_corners]
    type = ExtraNodesetGenerator
    input = gmg
    new_boundary = tip_corners
    coord = '0.0 10.0 0.0; 1.0 10.0 0.0'
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

[AuxVariables]
  [moment_22]
    order = CONSTANT
    family = MONOMIAL
  []
  [moment_22_nodal]
    order = FIRST
    family = LAGRANGE
  []
[]

[AuxKernels]
  [moment_22]
    type = DSGShellResultantsAux
    variable = moment_22
    resultant = bending_moment_22
    execute_on = TIMESTEP_END
  []
  [moment_22_nodal]
    type = DSGShellNodalResultantsAux
    variable = moment_22_nodal
    resultant = bending_moment_22
    nodal_resultants = nodal_resultants
    execute_on = TIMESTEP_END
  []
[]

[UserObjects]
  [nodal_resultants]
    type = DSGShellNodalResultants
    execute_on = TIMESTEP_END
  []
[]

[BCs]
  [clamp_disp_x]
    type = DirichletBC
    variable = disp_x
    boundary = bottom
    value = 0.0
  []
  [clamp_disp_y]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0.0
  []
  [clamp_disp_z]
    type = DirichletBC
    variable = disp_z
    boundary = bottom
    value = 0.0
  []
  [clamp_rot_x]
    type = DirichletBC
    variable = rot_x
    boundary = bottom
    value = 0.0
  []
  [clamp_rot_y]
    type = DirichletBC
    variable = rot_y
    boundary = bottom
    value = 0.0
  []
  [clamp_rot_z]
    type = DirichletBC
    variable = rot_z
    boundary = bottom
    value = 0.0
  []
[]

[NodalKernels]
  [tip_load]
    type = ConstantRate
    variable = disp_z
    boundary = tip_corners
    rate = 0.5
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
    compute_nodal_strains = true
  []
  [section]
    type = ComputeDSGShellSectionStiffness
    youngs_modulus = 2100000
    poissons_ratio = 0.0
    thickness = 0.1
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
  nl_abs_tol = 1e-9
  dt = 1
  dtmin = 1
  end_time = 1
[]

[Postprocessors]
  [moment_22_clamp]
    type = PointValue
    point = '0.0 0.0 0.0'
    variable = moment_22_nodal
  []
  [moment_22_y_2_5]
    type = PointValue
    point = '0.0 2.5 0.0'
    variable = moment_22_nodal
  []
  [moment_22_y_5]
    type = PointValue
    point = '0.0 5.0 0.0'
    variable = moment_22_nodal
  []
  [moment_22_y_7_5]
    type = PointValue
    point = '0.0 7.5 0.0'
    variable = moment_22_nodal
  []
  [moment_22_tip]
    type = PointValue
    point = '0.0 10.0 0.0'
    variable = moment_22_nodal
  []
[]

[Outputs]
  csv = true
[]
