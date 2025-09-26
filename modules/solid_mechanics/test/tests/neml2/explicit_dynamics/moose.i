!include 'expdyn.i'

[Kernels]
  [sdx]
    type = StressDivergenceTensors
    variable = disp_x
    component = 0
  []
  [sdy]
    type = StressDivergenceTensors
    variable = disp_y
    component = 1
  []
  [sdz]
    type = StressDivergenceTensors
    variable = disp_z
    component = 2
  []
[]

[Materials]
  [C]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1
    poissons_ratio = 0.3
  []
  [strain]
    type = ComputeSmallStrain
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
    second_order_vars = 'disp_x disp_y disp_z'
  []

  start_time = 0.0
  num_steps = 30
  dt = 0.02
[]
