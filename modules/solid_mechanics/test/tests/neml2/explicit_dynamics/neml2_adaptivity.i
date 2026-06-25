!include 'expdyn.i'

[NEML2]
  eager = true
  input = '../elasticity/elasticity_neml2.i'
  [all]
    executor_name = 'neml2'
    model = 'model'
    verbose = true
    input_kernels = 'neml2_strain'
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
  [neml2_strain]
    type = NEML2SmallStrain
    assembly = 'assembly'
    fe = 'fe'
    to_neml2 = 'neml2_strain'
  []
  [residual]
    type = NEML2StressDivergence
    assembly = 'assembly'
    fe = 'fe'
    executor = 'neml2'
    stress = 'neml2_stress'
    residual = 'NONTIME'
  []
[]

[Adaptivity]
  [Markers]
    [uniform]
      type = UniformMarker
      mark = REFINE
    []
  []
  marker = uniform
  interval = 13
[]

[Executioner]
  type = Transient

  [TimeIntegrator]
    type = NEML2CentralDifference
    mass_matrix_tag = 'mass'
    use_constant_mass = true
    recompute_mass_matrix_after_mesh_change = true
    second_order_vars = 'disp_x disp_y disp_z'
    assembly = 'assembly'
    fe = 'fe'
  []

  start_time = 0.0
  num_steps = 30
  dt = 0.02
[]
