# The tank of tank_shell.i with its axis inclined at 45 degrees in the XZ plane (the
# tank_shell_rotated case of the module's shell battery): the same hoop force p r = 20 MN/m
# and radial displacement p r^2 / (E t) = 1.6667e-3 m, read at the mid-height node of the
# generatrix in the XZ plane, where the radial direction is (1, 0, -1) / sqrt(2). The first
# local axis follows the inclined axis of the tank, and the normals of this mesh point into
# the tank as in tank_shell.i, so the positive pressure acts outward.

[Mesh]
  [tank]
    type = FileMeshGenerator
    file = tank_shell_rotated.msh
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
  [membrane_force_11]
    order = CONSTANT
    family = MONOMIAL
  []
  [membrane_force_22]
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
[]

[BCs]
  [support_disp_x]
    type = DirichletBC
    variable = disp_x
    boundary = lower_circle
    value = 0.0
  []
  [support_disp_y]
    type = DirichletBC
    variable = disp_y
    boundary = lower_circle
    value = 0.0
  []
  [support_disp_z]
    type = DirichletBC
    variable = disp_z
    boundary = lower_circle
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
  [pressure_disp_x]
    type = DistributedLoadDSGShell
    variable = disp_x
    displacements = 'disp_x disp_y disp_z'
    function = '40'
    project_load_to_normal = true
  []
  [pressure_disp_y]
    type = DistributedLoadDSGShell
    variable = disp_y
    displacements = 'disp_x disp_y disp_z'
    function = '40'
    project_load_to_normal = true
  []
  [pressure_disp_z]
    type = DistributedLoadDSGShell
    variable = disp_z
    displacements = 'disp_x disp_y disp_z'
    function = '40'
    project_load_to_normal = true
  []
[]

[Materials]
  [strain]
    type = ComputeDSGShellStrain
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
    reference_first_local_direction = '0.70710678118654752 0 0.70710678118654752'
  []
  [section]
    type = ComputeDSGShellSectionStiffness
    youngs_modulus = 2e5
    poissons_ratio = 0.3
    thickness = 0.03
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
  nl_abs_tol = 1e-8
  dt = 1.0
  dtmin = 1.0
  end_time = 1.0
[]

[Postprocessors]
  [disp_x_mid]
    type = PointValue
    point = '1.4142135623730951 0.0 0.7071067811865476'
    variable = disp_x
    outputs = none
  []
  [disp_z_mid]
    type = PointValue
    point = '1.4142135623730951 0.0 0.7071067811865476'
    variable = disp_z
    outputs = none
  []
  [radial_disp_mid]
    type = ParsedPostprocessor
    expression = '(disp_x_mid - disp_z_mid) / sqrt(2)'
    pp_names = 'disp_x_mid disp_z_mid'
  []
  [hoop_force_mid]
    type = PointValue
    point = '1.4142135623730951 0.0 0.7071067811865476'
    variable = membrane_force_22
  []
  [axial_force_mid]
    type = PointValue
    point = '1.4142135623730951 0.0 0.7071067811865476'
    variable = membrane_force_11
  []
[]

[Outputs]
  csv = true
[]
