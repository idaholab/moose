# Bending of a cantilever strip modeled with DSG shell elements.
#
# A strip of length 10 m (along Y) and cross section 1 m x 0.1 m is modeled with
# four four-node elements along its length (Figure 6a of Dvorkin and Bathe, 1984).
# All degrees of freedom are fixed on the bottom edge. E = 2100000 and nu = 0.
# A load of 0.5 N in Z is applied at each of the two corner nodes of the top
# edge, a total load P = 1 N.
#
# The analytic tip deflection of a shear-deformable cantilever is
#   w = P L^3 / (3 E I) + P L / (k A G) = 1.904762 + 0.000114 = 1.904876 m
# with the shear correction factor k = 5/6, and the analytic tip slope is
#   P L^2 / (2 E I) = 0.2857143.
# The bending moment is P (L - y) in magnitude; the elemental average of the first element is
# its value at the element center, P (L - 1.25) = 8.75 N m per 1 m of width, reported as
# -8.75 since the loaded face, the one the normal points to, is in compression (the moment
# convention of ShellResultantsAux). The shear force is P = 1 N.
#
# The other members of the family run from this input through the command line
# of the test specification: the element type of the mesh generator and the
# order of the six variables (SECOND on the six-node and eight-node elements).

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
  [shear_2]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [moment_22]
    type = DSGShellResultantsAux
    variable = moment_22
    resultant = bending_moment_22
    execute_on = TIMESTEP_END
  []
  [shear_2]
    type = DSGShellResultantsAux
    variable = shear_2
    resultant = shear_force_2
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
  [disp_z_tip]
    type = PointValue
    point = '1.0 10.0 0.0'
    variable = disp_z
  []
  [rot_x_tip]
    type = PointValue
    point = '0.0 10.0 0.0'
    variable = rot_x
  []
  [moment_22_first_element]
    type = ElementalVariableValue
    elementid = 0
    variable = moment_22
  []
  [shear_2_first_element]
    type = ElementalVariableValue
    elementid = 0
    variable = shear_2
  []
[]

[Outputs]
  csv = true
[]
