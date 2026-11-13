# Scordelis-Lo roof, DSG shell elements.
#
# The benchmark of MacNeal and Harder, "A proposed standard set of problems to
# test finite element accuracy", Finite Elements in Analysis and Design, 1985.
# A cylindrical roof of radius 25 m, length 50 m and 80 degrees of opening,
# with E = 4.32e8, nu = 0 and thickness 0.25 m, carries its self weight of
# 90 per unit area downward. The straight edges are free. The curved ends
# rest on rigid diaphragms (no displacement in their own plane). One half of
# the roof is modeled, from the symmetry plane y = 0 to the diaphragm at
# y = 25, with a 15 x 15 structured mesh.
#
# The reference vertical deflection at the midpoint of the free edge is
# 0.3086 m downward. The mesh file is the one of the module's shell battery.

[Mesh]
  [file]
    type = FileMeshGenerator
    file = scordelis_lo_roof_shell.msh
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

[BCs]
  [diaphragm_x]
    type = DirichletBC
    variable = disp_x
    boundary = back
    value = 0.0
  []
  [diaphragm_z]
    type = DirichletBC
    variable = disp_z
    boundary = back
    value = 0.0
  []
  [symmetry_y]
    type = DirichletBC
    variable = disp_y
    boundary = front
    value = 0.0
  []
  [symmetry_rot_x]
    type = DirichletBC
    variable = rot_x
    boundary = front
    value = 0.0
  []
  [symmetry_rot_z]
    type = DirichletBC
    variable = rot_z
    boundary = front
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
  [self_weight]
    type = DistributedLoadDSGShell
    variable = disp_z
    displacements = 'disp_x disp_y disp_z'
    function = '90'
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
    youngs_modulus = 4.32e8
    poissons_ratio = 0.0
    thickness = 0.25
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
  [disp_z_free_edge]
    type = PointValue
    point = '-16.7 0.0 19.2'
    variable = disp_z
  []
[]

[Outputs]
  csv = true
[]
