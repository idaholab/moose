[Mesh]
    [gen]
        type = GeneratedMeshGenerator
        dim = 2
        xmin = 0
        ymin = 0
        xmax = 0.2
        ymax = 0.5
        nx = 5
        ny = 15
        elem_type = QUAD4
    []
    displacements = 'disp_x disp_y'
[]


[Problem]
    kernel_coverage_check = false
    skip_nl_system_check = true
[]

[AuxVariables]
    [solid_indicator]
        initial_condition = 1.0
    []
    [disp_x]
    []
    [disp_y]
    []
[]

[AuxKernels]
  [move]
    type = FunctionAux
    variable = disp_x
    function = 't'
  []
[]

[Executioner]
    type = Transient
    num_steps = 1
    solve_type = 'NEWTON'
    petsc_options_iname = '-pc_type -pc_factor_mat_solver_type -pc_factor_shift_type'
    petsc_options_value = 'lu       superlu_dist               NONZERO'
    nl_max_its = 40
    l_max_its = 15
    line_search = 'none'
    nl_abs_tol = 1e-5
    nl_rel_tol = 1e-4
    automatic_scaling = true
[]

[Outputs]
    [out]
        type = Exodus
        execute_on = 'INITIAL TIMESTEP_END'
    []
[]
