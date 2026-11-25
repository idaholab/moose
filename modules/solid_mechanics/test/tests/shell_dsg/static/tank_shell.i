# Cylindrical tank under internal pressure, DSG shell elements (the tank_shell case of
# the module's shell battery).
#
# A tank of radius r = 0.5 m, length 3 m along Z and wall thickness t = 0.03 m, E = 2e5 MPa
# and nu = 0.3, is held at its lower circle (displacements only) and loaded by an internal
# pressure p = 40 MPa, applied by DistributedLoadDSGShell against the element normals, the
# convention of the module's Pressure boundary condition. The normals of this mesh point
# into the tank (they follow the node ordering of the elements), so a positive pressure acts
# outward, an internal pressure. The upper circle is free, so the wall carries no axial force.
#
# Away from the held base the wall is in a membrane state: the hoop force is p r = 20 MN/m
# whatever the material, and the radial displacement is p r^2 / (E t) = 1.6667e-3 m. The
# first local axis is the axis of the tank, so the hoop force is the second membrane
# force; both are read at mid-height, together with the radial displacement there.

[Mesh]
  [tank]
    type = FileMeshGenerator
    file = tank_shell.msh
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
    reference_first_local_direction = '0 0 1'
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
  [radial_disp_mid]
    type = PointValue
    point = '0.5 0.0 1.5'
    variable = disp_x
  []
  [hoop_force_mid]
    type = PointValue
    point = '0.5 0.0 1.5'
    variable = membrane_force_22
  []
  [axial_force_mid]
    type = PointValue
    point = '0.5 0.0 1.5'
    variable = membrane_force_11
  []
[]

[Outputs]
  csv = true
[]
