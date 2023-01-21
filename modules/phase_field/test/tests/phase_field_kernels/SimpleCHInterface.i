#
# Test the non-split parsed function free enery Cahn-Hilliard kernel
# The free energy used here has the same functional form as the CHPoly kernel
# If everything works, the output of this test should replicate the output
# of marmot/tests/chpoly_test/CHPoly_test.i (exodiff match)
#

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 16
  ny = 16
  xmax = 50
  ymax = 50
  elem_type = QUAD4
[]

[Variables]
  [./cv]
    order = THIRD
    family = HERMITE
  [../]
[]

[ICs]
  [./InitialCondition]
    type = CrossIC
    x1 = 5.0
    y1 = 5.0
    x2 = 45.0
    y2 = 45.0
    variable = cv
  [../]
[]

[Kernels]
  [./ie_c]
    type = TimeDerivative
    variable = cv
  [../]
  [./CHSolid]
    type = CahnHilliard
    variable = cv
    f_name = F
    mob_name = M
  [../]
  [./CHInterface]
    type = SimpleCHInterface
    variable = cv
    mob_name = M
    kappa_name = kappa_c
  [../]
[]

[Materials]
  [./consts]
    type = GenericConstantMaterial
    prop_names  = 'M kappa_c'
    prop_values = '1 0.1'
  [../]
  [./free_energy]
    type = DerivativeParsedMaterial
    property_name = F
    coupled_variables = 'cv'
    expression = '(1-cv)^2 * (1+cv)^2'
  [../]
[]

[Executioner]
  type = Transient
  scheme = 'bdf2'

  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart'
  petsc_options_value = 'hypre boomeramg 31'

  l_max_its = 15
  l_tol = 1.0e-4
  nl_max_its = 10
  nl_rel_tol = 1.0e-11

  start_time = 0.0
  num_steps = 2
  dt = 0.7
[]

[Outputs]
  [./out]
    type = Exodus
    refinements = 1
  [../]
[]
