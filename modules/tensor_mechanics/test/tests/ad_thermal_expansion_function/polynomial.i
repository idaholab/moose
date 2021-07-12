# This test checks the thermal expansion calculated via a mean thermal expansion coefficient.
# The coefficient is selected so as to result in a 1e-4 strain in the x-axis, and to cross over
# from positive to negative strain.

[Mesh]
  [./gen]
    type = GeneratedMeshGenerator
    dim = 3
  [../]
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[AuxVariables]
  [./temp]
    initial_condition = 300
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
    boundary = left
    value = 0.0
  [../]

  [./bottom]
    type = ADDirichletBC
    variable = disp_y
    boundary = bottom
    value = 0.0
  [../]

  [./back]
    type = ADDirichletBC
    variable = disp_z
    boundary = back
    value = 0.0
  [../]
[]

[AuxKernels]
  [./temp]
    type = FunctionAux
    variable = temp
    function = '(1 + t) * 300'
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
    type = ADComputeMeanThermalExpansionPolynomialEigenstrain
    thermal_expansion_coefficients = '1e-6 1e-9 1e-11'
    thermal_expansion_function_reference_temperature = 300
    stress_free_temperature = 300
    temperature = temp
    eigenstrain_name = eigenstrain
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
  [./exact]
    type = ParsedPostprocessor
    pp_names = temp_avg
    function = '(1e-6 + 1e-9 * temp_avg + 1e-11 * temp_avg^2) * (temp_avg - 300)'
  [../]
[]

[Executioner]
  type = Transient

  end_time = 1.0
  dt = 0.1
[]

[Outputs]
  csv = true
[]
