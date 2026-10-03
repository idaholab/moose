!include 'expdyn_rz.i'

[Kernels]
  # use_displaced_mesh = false: integrate on the reference configuration to
  # match the NEML2 force path, which caches the assembly geometry once.
  # (StressDivergenceRZTensors defaults to the displaced mesh.)
  [sdx]
    type = StressDivergenceRZTensors
    variable = disp_x
    component = 0
    use_displaced_mesh = false
  []
  [sdy]
    type = StressDivergenceRZTensors
    variable = disp_y
    component = 1
    use_displaced_mesh = false
  []
[]

[Materials]
  [C]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1
    poissons_ratio = 0.3
  []
  [strain]
    type = ComputeAxisymmetricRZSmallStrain
    implicit = false
  []
  [stress]
    type = ComputeLinearElasticStress
    implicit = false
  []
[]

[Executioner]
  type = Transient

  [TimeIntegrator]
    type = ExplicitMixedOrder
    mass_matrix_tag = 'mass'
    use_constant_mass = true
    second_order_vars = 'disp_x disp_y'
  []

  start_time = 0.0
  num_steps = 30
  dt = 0.02
[]
