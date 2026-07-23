!include 'expdyn.i'

[NEML2]
  eager = true
  input = '../elasticity/elasticity_neml2.i'
  [all]
    executor_name = 'neml2'
    model = 'model'
    input_kernels = 'neml2_strain'
    auto_output = false
  []
[]

[UserObjects]
  [assembly]
    type = TorchAssembly
  []
  [fe]
    type = TorchFEInterpolation
    assembly = 'assembly'
  []
  [neml2_strain]
    type = TorchSmallStrain
    assembly = 'assembly'
    fe = 'fe'
    to_neml2 = 'neml2_strain'
  []
  [residual]
    type = TorchStressDivergence
    assembly = 'assembly'
    fe = 'fe'
    executor = 'neml2'
    stress = 'neml2_stress'
    residual = 'NONTIME'
  []
[]

[Executioner]
  type = Transient

  [TimeIntegrator]
    type = TorchCentralDifference
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
