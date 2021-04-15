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
  [u_0]
    order = FIRST
    family = L2_LAGRANGE
  []
  [u_1]
    order = FIRST
    family = L2_LAGRANGE
  []
[]

[AuxVariables]
  [u_diff_save_in_0]
    order = FIRST
    family = L2_LAGRANGE
  []
  [u_diff_save_in_1]
    order = FIRST
    family = L2_LAGRANGE
  []
  [u_vacuum_save_in_0]
    order = FIRST
    family = L2_LAGRANGE
  []
  [u_vacuum_save_in_1]
    order = FIRST
    family = L2_LAGRANGE
  []
  [u_dg_save_in_0]
    order = FIRST
    family = L2_LAGRANGE
  []
  [u_dg_save_in_1]
    order = FIRST
    family = L2_LAGRANGE
  []

  [u_diff_diag_save_in_0]
    order = FIRST
    family = L2_LAGRANGE
  []
  [u_diff_diag_save_in_1]
    order = FIRST
    family = L2_LAGRANGE
  []
  [u_vacuum_diag_save_in_0]
    order = FIRST
    family = L2_LAGRANGE
  []
  [u_vacuum_diag_save_in_1]
    order = FIRST
    family = L2_LAGRANGE
  []
  [u_dg_diag_save_in_0]
    order = FIRST
    family = L2_LAGRANGE
  []
  [u_dg_diag_save_in_1]
    order = FIRST
    family = L2_LAGRANGE
  []
[]

[Kernels]
  [diff0]
    type = MatCoefDiffusion
    variable = u_0
    conductivity = dc
    save_in = u_diff_save_in_0
    diag_save_in = u_diff_diag_save_in_0
  []
  [diff1]
    type = Diffusion
    variable = u_1
    save_in = u_diff_save_in_1
    diag_save_in = u_diff_diag_save_in_1
  []
  [reaction0]
    type = CoefReaction
    variable = u_0
  []
  [reaction1]
    type = CoefReaction
    variable = u_1
  []
  [reaction01]
    type = CoupledForce
    variable = u_1
    v = u_0
    coef = 0.1
  []
[]

[DGKernels]
  [dgdiff0]
    type = DGDiffusion
    variable = u_0
    diff = dc
    sigma = 4
    epsilon = 1
    save_in = u_dg_save_in_0
    diag_save_in = u_dg_diag_save_in_0
  []
  [dgdiff1]
    type = DGDiffusion
    variable = u_1
    sigma = 4
    epsilon = 1
    save_in = u_dg_save_in_1
    diag_save_in = u_dg_diag_save_in_1
  []
[]

[BCs]
  [left0]
    type = VacuumBC
    variable = u_0
    boundary = 1
    save_in = u_vacuum_save_in_0
    diag_save_in = u_vacuum_diag_save_in_0
  []
  [left1]
    type = VacuumBC
    variable = u_1
    boundary = 1
    save_in = u_vacuum_save_in_1
    diag_save_in = u_vacuum_diag_save_in_1
  []

  [right0]
    type = PenaltyDirichletBC
    variable = u_0
    boundary = 2
    value = 1
    penalty = 4
  []
  [right1]
    type = PenaltyDirichletBC
    variable = u_1
    boundary = 2
    value = 2
    penalty = 4
  []
[]

[Materials]
  [dc0]
    type = GenericConstantMaterial
    block = 0
    prop_names = dc
    prop_values = 1
  []
  [dc1]
    type = GenericConstantMaterial
    block = 1
    prop_names = dc
    prop_values = 2
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
    type = ElementIntegralVariablePostprocessor
    variable = u_0
  []
  [intu1]
    type = ElementIntegralVariablePostprocessor
    variable = u_1
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
[]

[Outputs]
  file_base = array_save_in_out
  exodus = true
[]
