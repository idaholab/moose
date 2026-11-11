# A unit cube whose right face is driven by a non-preset DirichletBC to disp_x = target, with the
# left face fixed. The first Newton step moves the right face by target. For target = -1.5 that
# step inverts the element (width 1 -> -0.5), so the damper must reduce it. For target = 1.5 the
# step only stretches the element (width 1 -> 2.5), so the damper must accept the full step.
target = -1.5

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  type = GeneratedMesh
  dim = 3
  elem_type = HEX8
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
  [disp_z]
  []
[]

[Kernels]
  [diff_x]
    type = Diffusion
    variable = disp_x
  []
  [diff_y]
    type = Diffusion
    variable = disp_y
  []
  [diff_z]
    type = Diffusion
    variable = disp_z
  []
[]

[BCs]
  [fix_x]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0
  []
  [fix_y]
    type = DirichletBC
    variable = disp_y
    boundary = 'left right'
    value = 0
  []
  [fix_z]
    type = DirichletBC
    variable = disp_z
    boundary = 'left right'
    value = 0
  []
  [drive_x]
    type = DirichletBC
    variable = disp_x
    boundary = right
    value = ${target}
    preset = false
  []
[]

[Dampers]
  [jac]
    type = ElementJacobianDamper
    # only an inverted element limits the step here
    max_increment = 1e6
    use_backtracking = true
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  line_search = none
  # Only the first Newton update is checked: the initial residual is 3, and a damped first update
  # leaves 1.5, which this tolerance accepts so the run ends after that update.
  nl_max_its = 1
  nl_abs_tol = 2
[]
