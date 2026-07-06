# Matched reference for neml2_large_def.i: the SAME St. Venant-Kirchhoff NEML2
# model through the conventional coupling — F gathered from the Lagrangian
# strain material's deformation_gradient property, PK2 returned as a material
# property, PK1 = F S formed by ComputeLagrangianStressCustomPK2, assembled by
# TotalLagrangianStressDivergence on the reference configuration.
!include 'expdyn.i'

[GlobalParams]
  large_kinematics = true
[]

[Functions]
  [forcing_fn]
    y := '0.0 0.0 0.05 0.2 0.35 0.4 0.4'
  []
[]

[NEML2]
  input = 'svk_neml2.i'
  [all]
    executor_name = 'neml2'
    model = 'model'
    verbose = true
    device = 'cpu'
    derivatives = 'neml2_stress deformation_gradient'
  []
[]

[Kernels]
  [sdx]
    type = TotalLagrangianStressDivergence
    variable = disp_x
    component = 0
  []
  [sdy]
    type = TotalLagrangianStressDivergence
    variable = disp_y
    component = 1
  []
  [sdz]
    type = TotalLagrangianStressDivergence
    variable = disp_z
    component = 2
  []
[]

[Materials]
  [strain]
    type = ComputeLagrangianStrain
  []
  [stress]
    type = ComputeLagrangianStressCustomPK2
    custom_pk2_stress = 'neml2_stress'
    custom_pk2_jacobian = 'dneml2_stress/ddeformation_gradient'
  []
[]

[Executioner]
  type = Transient

  [TimeIntegrator]
    type = ExplicitMixedOrder
    mass_matrix_tag = 'mass'
    use_constant_mass = true
    second_order_vars = 'disp_x disp_y disp_z'
  []

  start_time = 0.0
  num_steps = 30
  dt = 0.02
[]
