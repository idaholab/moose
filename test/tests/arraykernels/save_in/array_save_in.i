[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 4
    ny = 4
  []
  [subdomain1]
    input = gen
    type = SubdomainBoundingBoxGenerator
    bottom_left = '0.5 0.5 0'
    top_right = '1 1 0'
    block_id = 1
  []
[]

[Variables]
  [u]
    order = FIRST
    family = L2_LAGRANGE
    components = 2
  []
[]

[AuxVariables]
  [u_diff_save_in]
    order = FIRST
    family = L2_LAGRANGE
    components = 2
  []
  [u_vacuum_save_in]
    order = FIRST
    family = L2_LAGRANGE
    components = 2
  []
  [u_dg_save_in]
    order = FIRST
    family = L2_LAGRANGE
    components = 2
  []

  [u_diff_diag_save_in]
    order = FIRST
    family = L2_LAGRANGE
    components = 2
  []
  [u_vacuum_diag_save_in]
    order = FIRST
    family = L2_LAGRANGE
    components = 2
  []
  [u_dg_diag_save_in]
    order = FIRST
    family = L2_LAGRANGE
    components = 2
  []
[]

[Kernels]
  [diff]
    type = ArrayDiffusion
    variable = u
    diffusion_coefficient = dc
    save_in = u_diff_save_in
    diag_save_in = u_diff_diag_save_in
  []
  [reaction]
    type = ArrayReaction
    variable = u
    reaction_coefficient = rc
  []
[]

[DGKernels]
  [dgdiff]
    type = ArrayDGDiffusion
    variable = u
    diff = dc
    save_in = u_dg_save_in
    diag_save_in = u_dg_diag_save_in
  []
[]

[BCs]
  [left]
    type = ArrayVacuumBC
    variable = u
    boundary = 1
    save_in = u_vacuum_save_in
    diag_save_in = u_vacuum_diag_save_in
  []

  [right]
    type = ArrayPenaltyDirichletBC
    variable = u
    boundary = 2
    value = '1 2'
    penalty = 4
  []
[]

[Materials]
  [dc0]
    type = GenericConstantArray
    block = 0
    prop_name = dc
    prop_value = '1 1'
  []
  [dc1]
    type = GenericConstantArray
    block = 1
    prop_name = dc
    prop_value = '2 1'
  []
  [rc]
    type = GenericConstant2DArray
    block = '0 1'
    prop_name = rc
    prop_value = '1 0; -0.1 1'
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Postprocessors]
  [intu0]
    type = ElementIntegralArrayVariablePostprocessor
    variable = u
    component = 0
  []
  [intu1]
    type = ElementIntegralArrayVariablePostprocessor
    variable = u
    component = 1
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
[]

[Outputs]
  exodus = true
[]
