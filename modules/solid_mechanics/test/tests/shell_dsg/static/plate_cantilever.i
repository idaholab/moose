# Cantilever plate under a constant end rotation, DSG shell elements.
#
# A plate of 10 m x 1 m in the XZ plane, E = 2e11, nu = 0 and thickness
# t = 0.133887 m (I = b t^3 / 12 = 2e-4 m^4 per unit width), is clamped
# against rotation and transverse displacement at its left edge and given a
# rotation of 0.05 about Y at its right edge.
#
# The bending moment is constant along the plate, M = E I theta / L = 2e5 N m
# per unit width, and the tip deflection is M L^2 / (2 E I) = 0.25 m.

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 1
  xmin = 0.0
  xmax = 10.0
  zmin = 0.0
  zmax = 1.0
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
  [clamp_rot_y]
    type = DirichletBC
    variable = rot_y
    boundary = left
    value = 0.0
  []
  [clamp_disp_x]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0.0
  []
  [clamp_disp_z]
    type = DirichletBC
    variable = disp_z
    boundary = left
    value = 0.0
  []
  [end_rotation]
    type = DirichletBC
    variable = rot_y
    boundary = right
    value = 0.05
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
    youngs_modulus = 2e11
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
  [disp_z_tip]
    type = PointValue
    point = '10.0 0.0 0.0'
    variable = disp_z
  []
  [moment_11_mid]
    type = ElementalVariableValue
    elementid = 5
    variable = moment_11
  []
  [shear_1_mid]
    type = ElementalVariableValue
    elementid = 5
    variable = shear_1
  []
[]

[Outputs]
  csv = true
[]
