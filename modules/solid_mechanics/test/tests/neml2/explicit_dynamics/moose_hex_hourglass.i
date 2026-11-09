!include 'expdyn3d.i'

[Kernels]
  [sdx]
    type = StressDivergenceTensors
    variable = disp_x
    component = 0
    use_displaced_mesh = false
  []
  [sdy]
    type = StressDivergenceTensors
    variable = disp_y
    component = 1
    use_displaced_mesh = false
  []
  [sdz]
    type = StressDivergenceTensors
    variable = disp_z
    component = 2
    use_displaced_mesh = false
  []
  [hgx]
    type = HourglassCorrectionHex8
    variable = disp_x
    penalty = 10
    shear_modulus = 1
  []
  [hgy]
    type = HourglassCorrectionHex8
    variable = disp_y
    penalty = 10
    shear_modulus = 1
  []
  [hgz]
    type = HourglassCorrectionHex8
    variable = disp_z
    penalty = 10
    shear_modulus = 1
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
  [Quadrature]
    type = GAUSS
    order = CONSTANT
  []
  start_time = 0.0
  num_steps = 30
  dt = 0.02
[]
