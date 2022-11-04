# This test checks the thermal expansion calculated via a mean thermal expansion coefficient.
# The coefficient is selected so as to result in a 1e-4 strain in the x-axis, and to cross over
# from positive to negative strain.

[Mesh]
  [./gen]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 4
    ny = 4
    nz = 4
  [../]
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Variables]
  [./temp]
  [../]
[]

[Kernels]
  [./temp_diff]
    type = ADDiffusion
    variable = temp
  [../]
[]

[Modules/TensorMechanics/Master]
  [./all]
    strain = SMALL
    add_variables = true
    eigenstrain_names = eigenstrain
    generate_output = 'strain_xx strain_yy strain_zz'
    use_automatic_differentiation = true
  [../]
[]

[BCs]
  [./left]
    type = ADDirichletBC
    variable = disp_x
    boundary = 'left'
    value = 0.0
  [../]

  [./bottom]
    type = ADDirichletBC
    variable = disp_y
    boundary = 'bottom'
    value = 0.0
  [../]

  [./back]
    type = ADDirichletBC
    variable = disp_z
    boundary = 'back'
    value = 0.0
  [../]

  [./temp]
    type = ADFunctionDirichletBC
    variable = temp
    boundary = 'front back top bottom left right'
    function = '1 + t'
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ADComputeIsotropicElasticityTensor
    youngs_modulus = 1
    poissons_ratio = 0.3
  [../]
  [./stress]
    type = ADComputeLinearElasticStress
  [../]
  [./thermal_expansion_strain]
    type = ADComputeMeanThermalExpansionFunctionEigenstrain
    thermal_expansion_function = cte_func_mean
    thermal_expansion_function_reference_temperature = 1
    stress_free_temperature = 1
    temperature = temp
    eigenstrain_name = eigenstrain
  [../]
[]

[Functions]
  [./cte_func_mean]
    type = ParsedFunction
    expression = '1e-6 + 1e-8 * t + 1e-8 * t^2 + exp(t) * 1e-2'
  [../]
[]

[Postprocessors]
  [./disp_x_max]
    type = SideAverageValue
    variable = disp_x
    boundary = right
  [../]
  [./temp_avg]
    type = ElementAverageValue
    variable = temp
  [../]
[]

[Executioner]
  type = Transient
  solve_type = NEWTON

  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

  num_steps = 1
[]

[Outputs]
  csv = true
[]
