#
# Simple pull test for cracking. This tests the option to prescribe the
# cracking strength using an AuxVariable. In this case, an elemental
# AuxVariable is used, and a function is used to prescribe its value.
# One of the elements is weaker than the others, so the crack localizes
# in that element.
#
[Mesh]
   file = plate.e
[]

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[AuxVariables]
  [./cracking_stress_fn]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./crack_flags2]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Functions]
  [./displ]
    type = PiecewiseLinear
    x = '0 0.1 0.2 0.3 0.4'
    y = '0 0.001 0 -0.001 0'
  [../]
  [./fstress]
    type = ParsedFunction
    expression = 'if(x > 0.667, 1.1e6, 1.2e6)'
  [../]
[]

[Modules/TensorMechanics/Master]
  [./all]
    strain = FINITE
    add_variables = true
    generate_output = 'stress_xx stress_yy stress_zz stress_xy stress_yz stress_zx strain_xx strain_yy strain_xy strain_yz'
    use_automatic_differentiation = true
  [../]
[]

[AuxKernels]
  [./cracking_stress_fn]
    type = FunctionAux
    variable = cracking_stress_fn
    function = fstress
    execute_on = initial
  [../]
  [./crack_flags2]
    type = ADMaterialRealVectorValueAux
    property = crack_flags
    variable = crack_flags2
   component = 2
  [../]
[]

[BCs]
  [./pull]
    type = ADFunctionDirichletBC
    variable = disp_x
    boundary = '3 4'
    function = displ
  [../]

  [./pin_x]
    type = ADDirichletBC
    variable = disp_x
    boundary =  '1 2'
    value = 0
  [../]
  [./pin_y]
    type = ADDirichletBC
    variable = disp_y
    boundary = '1 4'
    value = 0.0
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ADComputeIsotropicElasticityTensor
    youngs_modulus = 200.0e7
    poissons_ratio = 0.0
  [../]
  [./elastic_stress]
    type = ADComputeSmearedCrackingStress
    cracking_stress = cracking_stress_fn
    softening_models = abrupt_softening
  [../]
  [./abrupt_softening]
    type = ADAbruptSoftening
    residual_stress = 0.0
  [../]
[]

[Postprocessors]
  [./elem_stress_xx]
    type = ElementalVariableValue
    variable = stress_xx
    elementid = 2
  [../]
  [./elem_strain_xx]
    type = ElementalVariableValue
    variable = strain_xx
    elementid = 2
  [../]
  [./elem_crack_flags_x]
    type = ElementalVariableValue
    variable = crack_flags2
    elementid = 2
  [../]
[]

[Executioner]
  type = Transient
  solve_type = Newton

  petsc_options_iname = '-ksp_gmres_restart'
  petsc_options_value = '101               '

  line_search = 'none'

  l_max_its = 100
  nl_max_its = 100
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-6
  l_tol = 1e-5
  start_time = 0.0
  end_time = 0.2
  dt = 0.0025
[]

[Outputs]
  exodus = true
[]
