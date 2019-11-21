xmax = 2
xscalar = 1

ymax = 0.5
yscalar = 1

[Mesh]
  [./gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 8
    ny = 8
    xmax = ${fparse xmax * xscalar}
    ymax = ${fparse ymax * yscalar}
  [../]
[]

[Problem]
  gradient_scaling_vector = '${fparse 1.0 / xscalar} ${fparse 1.0 / yscalar} 1'
[]

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Modules/TensorMechanics/Master]
  [./tm]
    add_variables = true
    additional_generate_output = 'vonmises_stress hydrostatic_stress stress_xx stress_yy stress_zz strain_xx strain_yy strain_zz'
    use_automatic_differentiation = true
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1e10
    poissons_ratio = 0.345
  [../]
  [./stress]
    type = ADComputeLinearElasticStress
  [../]
[]

[BCs]
  [./left]
    type = ADPresetBC
    boundary = 'left'
    variable = disp_x
    value = 0.0
  [../]
  [./right]
    type = ADPresetBC
    boundary = right
    variable = disp_x
    value = ${fparse 1e-3 * xscalar^3}
  [../]
  [./bottom]
    type = ADPresetBC
    boundary = 'bottom'
    variable = disp_y
    value = 0.0
  [../]
  [./top]
    type = ADPresetBC
    boundary = top
    variable = disp_y
    value = ${fparse 5e-4 * yscalar^3}
  [../]
[]

[Executioner]
  type = Steady

  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  line_search = 'none'

  automatic_scaling = true
  compute_scaling_once = false
[]

[Postprocessors]
  [./von_mises_avg]
    type = ElementAverageValue
    variable = vonmises_stress
  [../]
  [./hydro_avg]
    type = ElementAverageValue
    variable = hydrostatic_stress
  [../]
  [./strain_xx_point]
    type = PointValue
    variable = strain_xx
    point = '${fparse xmax / xscalar / 4} ${fparse ymax / yscalar / 4} 0'
  [../]
  [./stress_xx_point]
    type = PointValue
    variable = stress_xx
    point = '${fparse xmax / xscalar / 4} ${fparse ymax / yscalar / 4} 0'
  [../]
[]

[Outputs]
  csv = true
[]
