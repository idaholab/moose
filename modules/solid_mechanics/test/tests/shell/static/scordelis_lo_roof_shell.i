
# This model is a widely used benchmark model denoted the Scordelis-Lo roof.
# The maximum z-deformation is compared with the value given in "Proposed Standard Set of Problems to Test Finite Element Accuracy, Finite Elements in Analysis and Design, 1985".
# Based on the existing analytical Solutions, maximum deflection of the roof should be 0.3086
# The model results in a maximum deflection of 0.3090 (assuming a 15*15 structured mesh)

[Mesh]
  [file]
    type = FileMeshGenerator
    file = scordelis_lo_roof_shell.msh
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
[]

[BCs]

  [simply_support_y]
    type = ADDirichletBC
    variable = disp_x
    boundary = 'back'
    value = 0.0
  []
  [simply_support_z]
    type = ADDirichletBC
    variable = disp_z
    boundary = 'back'
    value = 0.0
  []

  [simply_support_x]
    type = ADDirichletBC
    variable = disp_y
    boundary = 'front'
    value = 0.0
  []

  [simply_rot_x]
    type = ADDirichletBC
    variable = rot_x
    boundary = 'front'
    value = 0.0
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
  petsc_options = '-snes_ksp_ew'
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = ' lu       mumps'
  line_search = 'none'
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-10
  dt = 1
  end_time = 1
[]

[Kernels]

  [solid_disp_x]
    type = ADStressDivergenceShell
    component = 0
    variable = disp_x
    through_thickness_order = SECOND
  []
  [solid_disp_y]
    type = ADStressDivergenceShell
    component = 1
    variable = disp_y
    through_thickness_order = SECOND
  []
  [solid_disp_z]
    type = ADStressDivergenceShell
    component = 2
    variable = disp_z
    through_thickness_order = SECOND
  []
  [solid_rot_x]
    type = ADStressDivergenceShell
    component = 3
    variable = rot_x
    through_thickness_order = SECOND
  []
  [solid_rot_y]
    type = ADStressDivergenceShell
    component = 4
    variable = rot_y
    through_thickness_order = SECOND
  []
  [self_weight]
    type = ADDistributedLoadShell
    factor = 90
    variable = disp_z
    displacements = 'disp_x disp_y disp_z'
  []
[]

[Materials]

  [elasticity_tshell]
    type = ADComputeIsotropicElasticityTensorShell
    youngs_modulus = 4.32e8
    poissons_ratio = 0.0
    through_thickness_order = SECOND
  []
  [strain_shell]
    type = ADComputeIncrementalShellStrain
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y'
    thickness = 0.25
    through_thickness_order = SECOND
  []
  [stress_shell]
    type = ADComputeShellStress
    through_thickness_order = SECOND
  []
[]

[Postprocessors]
  [disp_z2]
    type = PointValue
    point = '-16.7 0  19.2'
    variable = disp_z
  []
[]

[Outputs]
  exodus = true
[]
