# This is a basic test of the material time step computed by the
# ComputeMultipleInelasticStress model. If no inelastic models
# are defined, the material time step should be the maximum
# value representable by a real number.

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1
  ny = 1
  nz = 1
  elem_type = HEX8
[]

[AuxVariables]
  [damage_index]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[Modules/TensorMechanics/Master]
  [all]
    strain = SMALL
    incremental = true
    add_variables = true
    generate_output = 'stress_xx strain_xx'
  []
[]

[BCs]
  [symmy]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0
  []
  [symmx]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0
  []
  [symmz]
    type = DirichletBC
    variable = disp_z
    boundary = back
    value = 0
  []
  [axial_load]
    type = DirichletBC
    variable = disp_x
    boundary = right
    value = 0.01
  []
[]

[Materials]
  [stress]
    type = ComputeMultipleInelasticStress
    inelastic_models = ''
  []
  [elasticity]
    type = ComputeIsotropicElasticityTensor
    poissons_ratio = 0.2
    youngs_modulus = 10e9
  []
[]

[Postprocessors]
  [stress_xx]
    type = ElementAverageValue
    variable = stress_xx
  []
  [strain_xx]
    type = ElementAverageValue
    variable = strain_xx
  []
  [time_step_limit]
    type = MaterialTimeStepPostprocessor
  []
[]

[Executioner]
  type = Transient

  l_max_its  = 50
  l_tol      = 1e-8
  nl_max_its = 20
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-8

  dt = 0.1
  dtmin = 0.001
  end_time = 1.1
  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 0.1
    growth_factor = 2.0
    cutback_factor = 0.5
    timestep_limiting_postprocessor = time_step_limit
  []
[]

[Outputs]
  csv=true
[]
