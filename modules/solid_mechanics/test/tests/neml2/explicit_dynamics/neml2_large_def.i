# Large-deformation (total Lagrangian) explicit dynamics through the NEML2
# nodal-force path: F gathered by NEML2DeformationGradient, PK1 assembled by
# NEML2StressDivergence against the cached reference-configuration quadrature.
!include 'expdyn.i'

[Functions]
  [forcing_fn]
    # 20x the small-strain test: ~40% stretch, genuinely finite deformation
    y := '0.0 0.0 0.05 0.2 0.35 0.4 0.4'
  []
[]

[NEML2]
  input = 'svk_neml2.i'
  [all]
    executor_name = 'neml2'
    model = 'model'
    verbose = true
    input_kernels = 'deformation_gradient'
    auto_output = false
  []
[]

[UserObjects]
  [assembly]
    type = NEML2Assembly
  []
  [fe]
    type = NEML2FEInterpolation
    assembly = 'assembly'
  []
  [deformation_gradient]
    type = NEML2DeformationGradient
    assembly = 'assembly'
    fe = 'fe'
    to_neml2 = 'deformation_gradient'
  []
  [residual]
    type = NEML2StressDivergence
    assembly = 'assembly'
    fe = 'fe'
    executor = 'neml2'
    stress = 'pk1'
    residual = 'NONTIME'
  []
[]

[Executioner]
  type = Transient

  [TimeIntegrator]
    type = NEML2CentralDifference
    mass_matrix_tag = 'mass'
    use_constant_mass = true
    second_order_vars = 'disp_x disp_y disp_z'
    assembly = 'assembly'
    fe = 'fe'
  []

  start_time = 0.0
  num_steps = 30
  dt = 0.02
[]
