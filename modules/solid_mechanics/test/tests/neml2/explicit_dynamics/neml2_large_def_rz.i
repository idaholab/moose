# Large-deformation (total Lagrangian) axisymmetric explicit dynamics through
# the NEML2 nodal-force path: F (including the hoop stretch 1 + u_r/r) gathered
# by NEML2DeformationGradientRZ, PK1 assembled by NEML2StressDivergenceRZ.
!include 'expdyn_rz.i'

[Functions]
  [forcing_fn]
    # 20x the small-strain test: finite hoop stretch at the outer surface
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
    type = NEML2DeformationGradientRZ
    assembly = 'assembly'
    fe = 'fe'
    to_neml2 = 'deformation_gradient'
  []
  [residual]
    type = NEML2StressDivergenceRZ
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
    second_order_vars = 'disp_x disp_y'
    assembly = 'assembly'
    fe = 'fe'
  []

  start_time = 0.0
  num_steps = 30
  dt = 0.02
[]
