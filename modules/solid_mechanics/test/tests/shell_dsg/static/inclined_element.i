# Axial stretch of a single inclined DSG shell element.
#
# The unit square element of axial_strain.i (E = 5, nu = 0, t = 0.1) is
# rotated by 45 degrees about the X axis, so that it lies in the plane spanned
# by X and the direction (0, cos 45, sin 45). Its bottom edge is fixed and its
# top edge is moved by 0.2 along that inclined direction, which is the second
# local axis of the element. The membrane force in the second local direction
# is E t epsilon = 0.1 per unit length, as for the flat element, and the other
# resultants are zero: the resultants are reported in the element frame and
# do not depend on the orientation of the element in space.

[Mesh]
  [square]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 1
    ny = 1
    xmin = 0.0
    xmax = 1.0
    ymin = 0.0
    ymax = 1.0
  []
  [incline]
    type = TransformGenerator
    input = square
    transform = ROTATE
    vector_value = '0 45 0'
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
  [membrane_force_11]
    order = CONSTANT
    family = MONOMIAL
  []
  [membrane_force_22]
    order = CONSTANT
    family = MONOMIAL
  []
  [membrane_force_12]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [membrane_force_11]
    type = DSGShellResultantsAux
    variable = membrane_force_11
    resultant = membrane_force_11
    execute_on = TIMESTEP_END
  []
  [membrane_force_22]
    type = DSGShellResultantsAux
    variable = membrane_force_22
    resultant = membrane_force_22
    execute_on = TIMESTEP_END
  []
  [membrane_force_12]
    type = DSGShellResultantsAux
    variable = membrane_force_12
    resultant = membrane_force_12
    execute_on = TIMESTEP_END
  []
[]

[BCs]
  [fix_x]
    type = DirichletBC
    variable = disp_x
    boundary = bottom
    value = 0.0
  []
  [fix_y]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0.0
  []
  [fix_z]
    type = DirichletBC
    variable = disp_z
    boundary = bottom
    value = 0.0
  []
  [fix_rot_x]
    type = DirichletBC
    variable = rot_x
    boundary = bottom
    value = 0.0
  []
  [fix_rot_y]
    type = DirichletBC
    variable = rot_y
    boundary = bottom
    value = 0.0
  []
  [fix_rot_z]
    type = DirichletBC
    variable = rot_z
    boundary = bottom
    value = 0.0
  []
  [stretch_y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = top
    function = '0.2 * cos(pi / 4) * t'
  []
  [stretch_z]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = top
    function = '0.2 * sin(pi / 4) * t'
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
    youngs_modulus = 5.0
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
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-12
  dt = 1.0
  dtmin = 1.0
  end_time = 1.0
[]

[Postprocessors]
  [membrane_force_11]
    type = ElementalVariableValue
    elementid = 0
    variable = membrane_force_11
  []
  [membrane_force_22]
    type = ElementalVariableValue
    elementid = 0
    variable = membrane_force_22
  []
  [membrane_force_12]
    type = ElementalVariableValue
    elementid = 0
    variable = membrane_force_12
  []
[]

[Outputs]
  csv = true
[]
