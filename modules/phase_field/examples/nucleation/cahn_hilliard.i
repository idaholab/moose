#
# Test the split parsed function free enery Cahn-Hilliard Bulk kernel
# The free energy used here has the same functional form as the SplitCHPoly kernel
# If everything works, the output of this test should replicate the output
# of marmot/tests/chpoly_test/CHPoly_Cu_Split_test.i (exodiff match)
#

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 120
  ny = 120
  xmin = 0
  xmax = 500
  ymin = 0
  ymax = 500
  elem_type = QUAD4
[]

#[Adaptivity]
#  max_h_level = 5
#  initial_steps = 2
#  initial_marker = marker
#  [./Indicators]
#    [./indicator]
#      type = GradientJumpIndicator
#      variable = c
#    [../]
#  [../]
#  [./Markers]
#    [./marker]
#      type = ErrorFractionMarker
#      indicator = indicator
#      refine = 0.9
#    [../]
#  [../]
#[]

[Variables]
  [./c]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
      type = RandomIC
      min = 0.2
      max = 0.21
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
    type = CoupledTimeDerivative
    variable = w
    v = c
  [../]
[]

[Materials]
  [./pfmobility]
    type = PFMobility
    block = 0
    kappa = 25
    mob = 1
  [../]

  [./chemical_free_energy]
    type = DerivativeParsedMaterial
    block = 0
    f_name = Fc
    args = 'c'
    constant_names       = 'barr_height  cv_eq'
    constant_expressions = '0.1          0'
    function = 16*barr_height*c^2*(1-c)^2 # +0.01*(c*plog(c,0.005)+(1-c)*plog(1-c,0.005))
    derivative_order = 2
    outputs = exodus
  [../]
  [./test]
    type = ParsedMaterial
    block = 0
    f_name = T
    args = c
    function = if(c>0.5,1.1,2.2)
    outputs = exodus
  [../]
  [./probability]
    type = ParsedMaterial
    block = 0
    f_name = P
    args = c
    function = c*1e-7
    outputs = exodus
  [../]
  [./nucleation]
    type = DiscreteNucleation
    block = 0
    f_name = Fn
    op_names  = c
    op_values = 0.95
    hold_time = 500
    penalty = 5
    probability = P # 0.5e-7
    outputs = exodus
  [../]

  [./free_energy]
    type = DerivativeSumMaterial
    derivative_order = 2
    block = 0
    args = c
    sum_materials = 'Fc Fn'
  [../]
[]

[Preconditioning]
  # active = ' '
  [./SMP]
    type = SMP
    full = true
  [../]
[]

[BCs]
  [./Periodic]
    [./all]
      auto_direction = 'x y'
    [../]
  [../]
[]

[Executioner]
  type = Transient
  scheme = bdf2

  solve_type = 'NEWTON'
  #petsc_options_iname = -pc_type
  #petsc_options_value = lu

  nl_max_its = 10
  l_tol = 1.0e-4
  nl_rel_tol = 1.0e-10
  nl_abs_tol = 1.0e-10
  start_time = 0.0
  num_steps = 1200

  dt = 10
  dtmin = 1e-6
[]

[Outputs]
  output_initial = true
  interval = 1
  exodus = true
  print_perf_log = true
[]
