#
# Test the split parsed function free enery Cahn-Hilliard Bulk kernel
# The free energy used here has the same functional form as the SplitCHPoly kernel
# If everything works, the output of this test should replicate the output
# of marmot/tests/chpoly_test/CHPoly_Cu_Split_test.i (exodiff match)
#

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 30
  ny = 30
  nz = 0
  xmin = 0
  xmax = 250
  ymin = 0
  ymax = 250
  zmin = 0
  zmax = 0
  elem_type = QUAD4
[]

[AuxVariables]
  [./phi]
  [../]
  [./a]
  [../]
  [./b]
  [../]
[]

[Variables]
  [./c]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
      type = SmoothCircleIC
      x1 = 125.0
      y1 = 125.0
      radius = 60.0
      invalue = 1.0
      outvalue = 0.1
      int_width = 30.0
    [../]
  [../]
  [./w]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./c_res]
    type = SplitCHParsed
    variable = c
    f_name = F
    kappa_name = kappa_c
    w = w
  [../]
  [./w_res]
    type = SplitCHWRes
    variable = w
    mob_name = M
  [../]
  [./time]
    type = CoupledImplicitEuler
    variable = w
    v = c
  [../]
[]

[Materials]
  [./constant]
    type = GenericConstantMaterial
    block = 0
    prop_names = 'constant_zero'
    prop_values = '0'
  [../]

  [./pfmobility]
    type = PFMobility
    block = 0
    kappa = 0.1
    mob = 1e-3
  [../]

  [./free_energy_a]
    type = DerivativeParsedMaterial
    block = 0
    f_name = Fa
    args = 'a b c'
    constant_names       = 'barr_height  cv_eq'
    constant_expressions = '0.1          1.0e-2'
    function = 16*barr_height*(c-cv_eq)^2*(1-cv_eq-c)^2+a+b
    third_derivatives = false
  [../]
  [./free_energy_b]
    type = DerivativeParsedMaterial
    block = 0
    f_name = Fb
    args = 'a b c'
    constant_names       = 'barr_height  cv_eq'
    constant_expressions = '0.1          1.0e-2'
    function = 16*barr_height*(c-cv_eq)^2*(1-cv_eq-c)^2-a-b
    #third_derivatives = false
  [../]

  [./free_energy]
    type = DerivativeTwoPhaseMaterial
    block = 0
    fa_name = Fa
    fb_name = Fb
    args = 'a b c'
    phi = phi
    outputs = exodus
    #third_derivatives = false
  [../]
[]

[Preconditioning]
  # active = ' '
  [./SMP]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  scheme = bdf2

  solve_type = 'NEWTON'
  petsc_options_iname = -pc_type
  petsc_options_value = lu

  l_max_its = 30
  l_tol = 1.0e-4
  nl_rel_tol = 1.0e-10
  start_time = 0.0
  num_steps = 1

  dt = 10
[]

[Outputs]
  output_initial = true
  interval = 1
  exodus = true

  [./console]
    type = Console
    perf_log = true
  [../]
[]
