starting_point = 2e-1

# We offset slightly so we avoid the case where the bottom of the secondary block and the top of the
# primary block are perfectly vertically aligned which can cause the backtracking line search some
# issues for a coarse mesh (basic line search handles that fine)
offset = 1e-2

[GlobalParams]
  displacements = 'disp_x disp_y'
  diffusivity = 1e0
  scaling = 1e0
[]

[Mesh]
  [original_file_mesh]
    type = FileMeshGenerator
    file = long-bottom-block-1elem-blocks-coarse.e
  []
  # These sidesets need to be deleted because the contact action adds them automatically. For this
  # particular mesh, the new IDs will be identical to the deleted ones and will conflict if we don't
  # remove the original ones.
  [delete_3]
    type = BlockDeletionGenerator
    input = original_file_mesh
    block = 3
  []
  [revised_file_mesh]
    type = BlockDeletionGenerator
    input = delete_3
    block = 4
  []
[]

[Variables]
  [disp_x]
    block = '1 2'
    # order = SECOND
  []
  [disp_y]
    block = '1 2'
    # order = SECOND
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

[ICs]
  [disp_y]
    block = 2
    variable = disp_y
    value = '${fparse starting_point + offset}'
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
  [topy]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 30
    function = '${starting_point} * cos(2 * pi / 40 * t) + ${offset}'
  []
  [leftx]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 50
    function = '1e-2 * t'
  []
[]

[Executioner]
  type = Transient
  end_time = 200
  dt = 5
  dtmin = .1
  solve_type = 'PJFNK'
  petsc_options = '-snes_converged_reason -ksp_converged_reason -pc_svd_monitor '
                  '-snes_linesearch_monitor -snes_ksp_ew'
  petsc_options_iname = '-pc_type -pc_factor_shift_type -pc_factor_shift_amount -mat_mffd_err'
  petsc_options_value = 'lu       NONZERO               1e-15                   1e-5'
  l_max_its = 30
  nl_max_its = 20
  line_search = 'none'
[]

[Debug]
  show_var_residual_norms = true
[]

[Outputs]
  exodus = true
  hide = 'contact_pressure nodal_area penetration'
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Postprocessors]
  [num_nl]
    type = NumNonlinearIterations
  []
  [cumulative]
    type = CumulativeValuePostprocessor
    postprocessor = num_nl
  []
  [contact]
    type = ContactDOFSetSize
    variable = frictional_normal_lm
    subdomain = frictional_secondary_subdomain
    execute_on = 'nonlinear timestep_end'
  []
[]
