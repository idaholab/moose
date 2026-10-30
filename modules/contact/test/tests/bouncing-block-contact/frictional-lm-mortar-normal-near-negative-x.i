starting_point = 2e-1

# We offset slightly so we avoid the case where the bottom of the secondary block and the top of the
# primary block are perfectly vertically aligned which can cause the backtracking line search some
# issues for a coarse mesh (basic line search handles that fine)
offset = 1e-2

# Rigid rotation of the whole problem that puts the secondary normal 0.1 degrees from -x
angle = -90.1
cos_angle = ${fparse cos(angle * pi / 180)}
sin_angle = ${fparse sin(angle * pi / 180)}

[GlobalParams]
  displacements = 'disp_x disp_y'
  diffusivity = 1e0
  scaling = 1e0
[]

[Mesh]
  [file_mesh]
    type = FileMeshGenerator
    file = long-bottom-block-1elem-blocks-coarse.e
  []
  # These subdomains need to be deleted because the contact action adds them automatically
  [delete_3]
    type = BlockDeletionGenerator
    input = file_mesh
    block = 3
  []
  [delete_4]
    type = BlockDeletionGenerator
    input = delete_3
    block = 4
  []
  [rotate]
    type = TransformGenerator
    input = delete_4
    transform = ROTATE
    vector_value = '${angle} 0 0'
  []
[]

[Variables]
  [disp_x]
    block = '1 2'
  []
  [disp_y]
    block = '1 2'
  []
[]

[ICs]
  [disp_x]
    block = 2
    variable = disp_x
    value = '${fparse -sin_angle * (starting_point + offset)}'
    type = ConstantIC
  []
  [disp_y]
    block = 2
    variable = disp_y
    value = '${fparse cos_angle * (starting_point + offset)}'
    type = ConstantIC
  []
[]

[Kernels]
  [disp_x]
    type = MatDiffusion
    variable = disp_x
  []
  [disp_y]
    type = MatDiffusion
    variable = disp_y
  []
[]

[Contact]
  [frictional]
    primary = 20
    secondary = 10
    formulation = mortar
    model = coulomb
    friction_coefficient = 0.1
    c_normal = 1.0e-2
    c_tangential = 1.0e-1
  []
[]

[BCs]
  [botx]
    type = DirichletBC
    variable = disp_x
    boundary = 40
    value = 0.0
  []
  [boty]
    type = DirichletBC
    variable = disp_y
    boundary = 40
    value = 0.0
  []
  # The top block is driven along the rotated normal and pulled along the rotated tangent
  [topx]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 30
    function = '${fparse -sin_angle} * (${starting_point} * cos(2 * pi / 40 * t) + ${offset}) + '
               '${cos_angle} * 1e-2 * t'
  []
  [topy]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 30
    function = '${cos_angle} * (${starting_point} * cos(2 * pi / 40 * t) + ${offset}) + '
               '${sin_angle} * 1e-2 * t'
  []
[]

[Executioner]
  type = Transient
  end_time = 15
  dt = 5
  # Any failed solve fails the test
  dtmin = 5
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type -pc_factor_shift_amount'
  petsc_options_value = 'lu       NONZERO               1e-15'
  nl_max_its = 20
  line_search = 'none'
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Postprocessors]
  [max_normal_lm]
    type = NodalExtremeValue
    variable = frictional_normal_lm
    block = frictional_secondary_subdomain
  []
[]

[Outputs]
  csv = true
[]
