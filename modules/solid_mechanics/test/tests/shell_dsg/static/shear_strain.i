# In-plane shear of a strip of DSG shell elements.
#
# A strip of 10 m x 1.5 m with E = 5 N/m^2, nu = 0 and thickness 0.1 m is
# fixed at its left edge and its right edge is moved by 0.05 m along the edge.
# The strip works as a short deep beam in its own plane; the membrane shear
# force of the elements and the reaction at the fixed edge are recorded.

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 4
  ny = 1
  xmin = 0.0
  xmax = 10.0
  ymin = 0.0
  ymax = 1.5
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
  [membrane_force_12]
    order = CONSTANT
    family = MONOMIAL
  []
  [membrane_force_11]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [membrane_force_12]
    type = DSGShellResultantsAux
    variable = membrane_force_12
    resultant = membrane_force_12
    execute_on = TIMESTEP_END
  []
  [membrane_force_11]
    type = DSGShellResultantsAux
    variable = membrane_force_11
    resultant = membrane_force_11
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
  [shear]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = right
    function = displacement
  []
[]

[Functions]
  [displacement]
    type = PiecewiseLinear
    x = '0.0 1.0'
    y = '0.0 0.05'
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
  [membrane_force_12_first]
    type = ElementalVariableValue
    elementid = 0
    variable = membrane_force_12
  []
  [membrane_force_12_last]
    type = ElementalVariableValue
    elementid = 3
    variable = membrane_force_12
  []
  [membrane_force_11_first]
    type = ElementalVariableValue
    elementid = 0
    variable = membrane_force_11
  []
  [disp_x_tip]
    type = PointValue
    point = '10.0 1.5 0.0'
    variable = disp_x
  []
[]

[Outputs]
  csv = true
[]
