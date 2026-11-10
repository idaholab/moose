# Axial strain and membrane force of a single DSG shell element.
#
# A 1 mm x 1 mm element with E = 5 N/mm^2, nu = 0 and thickness 0.1 mm is
# fixed at its left edge and stretched by 0.2 mm at its right edge. The axial
# strain is 0.2 and the membrane force per unit length is E t epsilon = 0.1 N/mm.

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
  [fix_x]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0.0
  []
  [fix_y]
    type = DirichletBC
    variable = disp_y
    boundary = left
    value = 0.0
  []
  [fix_z]
    type = DirichletBC
    variable = disp_z
    boundary = left
    value = 0.0
  []
  [fix_rot_x]
    type = DirichletBC
    variable = rot_x
    boundary = left
    value = 0.0
  []
  [fix_rot_y]
    type = DirichletBC
    variable = rot_y
    boundary = left
    value = 0.0
  []
  [fix_rot_z]
    type = DirichletBC
    variable = rot_z
    boundary = left
    value = 0.0
  []
  [stretch]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = right
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
  [disp_x_right]
    type = PointValue
    point = '1.0 1.0 0.0'
    variable = disp_x
  []
[]

[Outputs]
  csv = true
[]
