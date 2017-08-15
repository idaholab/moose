#
# Test to exercise the exponential stress release
#
# Stress vs. strain should show a linear relationship until cracking,
#   an exponential stress release, a linear relationship back to zero
#   strain, a linear relationship with the original stiffness in
#   compression and then back to zero strain, a linear relationship
#   back to the exponential curve, and finally further exponential
#   stress release.
#

[Mesh]
  file = crack-mesh.e
  displacements = 'disp_x disp_y'
  uniform_refine = 0
[]

[Variables]

  [./disp_x]
    order = FIRST
    family = LAGRANGE
  [../]

  [./disp_y]
    order = FIRST
    family = LAGRANGE
  [../]

[]

[AuxVariables]

  [./strain_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./strain_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./strain_xy]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./stress_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./stress_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./stress_xy]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./resid_x]
    order = FIRST
    family = LAGRANGE
#    block = 1
  [../]
  [./resid_y]
    order = FIRST
    family = LAGRANGE
#    block = 1
  [../]

  [./damage]
    order = CONSTANT
    family = MONOMIAL
  [../]

[]

[Functions]
  [./tfunc]
    type = ParsedFunction
    value = '0.001 * t'
  [../]
[]

[SolidMechanics]
  [./solid]
    disp_x = disp_x
    disp_y = disp_y
    save_in_disp_x = resid_x
    save_in_disp_y = resid_y
  [../]
[]


[AuxKernels]
  [./strain_xx]
    type = MaterialTensorAux
    variable = strain_xx
    tensor = total_strain
    index = 0
  [../]

  [./strain_yy]
    type = MaterialTensorAux
    variable = strain_yy
    tensor = total_strain
    index = 1
  [../]

  [./strain_xy]
    type = MaterialTensorAux
    variable = strain_xx
    tensor = total_strain
    index = 3
  [../]

  [./stress_xx]
    type = MaterialTensorAux
    variable = stress_xx
    tensor = stress
    index = 0
  [../]

  [./stress_yy]
    type = MaterialTensorAux
    variable = stress_yy
    tensor = stress
    index = 1
  [../]

  [./stress_xy]
    type = MaterialTensorAux
    variable = stress_xy
    tensor = stress
    index = 3
  [../]

  [./damage]
    type = MaterialStdVectorAux
    variable = damage
    property = intvar
    index = 0
  [../]

[]


[BCs]
  [./ydisp]
    type = FunctionPresetBC
    variable = disp_y
    boundary = 3
    function = tfunc
  [../]
  [./yfix]
    type = PresetBC
    variable = disp_y
    boundary = 2
    value = 0.0
  [../]
  [./xfix]
    type = PresetBC
    variable = disp_x
    boundary = 1
    value = 0
  [../]
[]

[Materials]
  [./elastic]
    type = SolidModel
    block = 1
    youngs_modulus = 186.5e9
    poissons_ratio = .316
    disp_x = disp_x
    disp_y = disp_y
    formulation = linear
    constitutive_model = crack
  [../]
  [./crack]
    type = RateDepSmearIsoCrackModel
    block = 1
    critical_energy = 1e6
    ref_damage_rate = 1e-3
    tol = 1e-5
    maxiter = 20
    exponent = 1.0
    nstate = 2
    intvar_incr_tol = 1000.0
    input_random_scaling_var = true
    random_scaling_var = 1e10
  [../]
[]

[Executioner]
  type = Transient

  # Preconditioned JFNK (default)
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -ksp_gmres_restart -sub_ksp_type -sub_pc_type -pc_asm_overlap'
  petsc_options_value = 'asm      31                  preonly       lu           1'

  line_search = 'none'
  nl_max_its = 20
  nl_rel_tol = 1e-15
  l_tol = 1e-1
  dt = 0.05
  dtmin = 0.05
  num_steps = 2
[]

[Outputs]
  exodus = true
  csv = true
  gnuplot = true
[]

[Postprocessors]
  [./resid_x]
    type = NodalSum
    variable = resid_x
    boundary = 3
  [../]
  [./resid_y]
    type = NodalSum
    variable = resid_y
    boundary = 3
  [../]
[]
