[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 10
  ny = 10
  nz = 10
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  zmin = 0
  zmax = 1
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Variables]
  [disp_x]
    scaling = 1e-10
  []
  [disp_y]
    scaling = 1e-10
  []
  [disp_z]
    scaling = 1e-10
  []
[]

[Materials]
  [./elasticity_tensor]
    type = ADComputeIsotropicElasticityTensor
    youngs_modulus = 1e3
    poissons_ratio = 0.3
    shear_modulus = 1e6
  [../]
  [./plasticity]
    type = ADHillPlasticityStressUpdate
    hardening_constant = 1000.0
    yield_stress = 0.100
    absolute_tolerance = 1e-14
    relative_tolerance = 1e-12
    base_name = plasticity
    max_inelastic_increment = 2.0e-6
    internal_solve_output_on = on_error
    acceptable_multiplier = 30.00
  [../]
  [./stress]
    type = ComputeFiniteStrainElasticStress
  [../]
[]

[Functions]
  [./temp_func]
    type = ParsedFunction
    value = '500 - 10 * t'
  [../]
[]

[BCs]
  [./fixed]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0.0
  [../]
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  num_steps = 20
  dt = 0.1
[]

[Postprocessors]
  [max_stress]
    type = ElementExtremeValue
    variable = vonmises_stress
  []
  [max_disp_x]
    type = ElementExtremeValue
    variable = disp_x
  []
  [max_disp_y]
    type = ElementExtremeValue
    variable = disp_y
  []
  [dt]
    type = TimestepSize
  []
[]

[Outputs]
  [./exodus]
    type = Exodus
    file_base = anis_plasticity_test2_out
  [../]
[]
