[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
  []
[]

[UserObjects]
  [u_patch]
    type = NodalPatchRecoveryMaterialProperty
    patch_polynomial_order = FIRST
    property = 'u'
    execute_on = 'TIMESTEP_END'
  []
[]

[Variables]
  [v]
    order = FIRST
    family = LAGRANGE
  []
[]

[Kernels]
  [dt]
    type = TimeDerivative
    variable = v
  []
  [diff]
    type = Diffusion
    variable = v
  []
[]

[AuxVariables]
  [u_recovered]
  []
  [u_nodal]
  []
  [u_diff]
  []
[]

[AuxKernels]
  [u_recovered]
    type = NodalPatchRecoveryAux
    variable = u_recovered
    nodal_patch_recovery_uo = u_patch
    execute_on = 'TIMESTEP_END'
  []
  [u_nodal]
    type = ParsedAux
    variable = u_nodal
    expression = v^2
    coupled_variables = v
  []
  [u_diff]
    type = ParsedAux
    variable = u_diff
    expression = u_nodal-u_recovered
    coupled_variables = 'u_nodal u_recovered'
  []
[]

[BCs]
  [fix_left]
    type = FunctionDirichletBC
    variable = v
    boundary = 'left'
    function = y+1
  []
  [fix_right]
    type = DirichletBC
    variable = v
    boundary = 'right'
    value = 0
  []
[]

[Materials]
  [u]
    type = ParsedMaterial
    expression = v^2
    property_name = u
    coupled_variables = v
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON

  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

  automatic_scaling = true

  dt = 0.4
  num_steps = 5

  nl_abs_tol = 1e-10
  nl_rel_tol = 1e-10
[]

[Outputs]
  exodus = true
  print_linear_residuals = false
[]
