!include 'expdyn.i'

[NEML2]
  input = '../elasticity/elasticity_neml2.i'
  [all]
    executor_name = 'neml2'
    model = 'model'
    verbose = true
    moose_input_kernels = 'strain'
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
  [strain]
    type = NEML2SmallStrain
    assembly = 'assembly'
    fe = 'fe'
    to_neml2 = 'forces/E'
  []
  [residual]
    type = NEML2StressDivergence
    assembly = 'assembly'
    fe = 'fe'
    executor = 'neml2'
    stress = 'state/S'
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
