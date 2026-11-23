[Mesh]
    [igafile]
        type = FileMeshGenerator
        file = 'three_edge2.xda'
        constraint_matrix = 'nasty_extraction.m'
        allow_renumbering = false
    []
[]

[Variables]
  [u]
    order = FIRST
  []
[]

[Kernels]
  [time]
    type = TimeDerivative
    variable = u
    block = 0  # Avoid direct calculations on spline nodes
  []
  [diff]
    type = Diffusion
    variable = u
    block = 0  # Avoid direct calculations on spline nodes
  []
  [null]
    type = NullKernel
    variable = u
    block = 1  # Keep kernel coverage check happy
  []
[]

[ICs]
  [u]
    type = FunctionIC
    variable = u
    function = 'sin(x)'
  []
[]

[Executioner]
    type = Transient
    petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
    petsc_options_value = 'lu mumps'

    num_steps = 0
    solve_type = NEWTON
    dtmin = 1
[]

[Postprocessors]
  [integral]
    type = ElementIntegralVariablePostprocessor
    variable = u
    block = 0
    execute_on = 'initial timestep_end'
  []
[]

[Outputs]
    csv = true
[]
