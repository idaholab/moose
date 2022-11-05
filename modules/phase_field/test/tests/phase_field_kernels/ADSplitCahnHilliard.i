#
# Test the split parsed function free enery Cahn-Hilliard Bulk kernel
# The free energy used here has the same functional form as the SplitCHPoly kernel
# If everything works, the output of this test should replicate the output
# of marmot/tests/chpoly_test/CHPoly_Cu_Split_test.i (exodiff match)
#

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  xmin = 0
  xmax = 60
  ymin = 0
  ymax = 60
  elem_type = QUAD4
[]

[Variables]
  [./c]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
      type = SmoothCircleIC
      x1 = 0
      y1 = 0
      radius = 30.0
      invalue = 1.0
      outvalue = -0.5
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
    type = ADSplitCHParsed
    variable = c
    f_name = F
    kappa_name = kappa_c
    w = w
  [../]
  [./w_res]
    type = ADSplitCHWRes
    variable = w
    mob_name = M
  [../]
  [./time]
    type = ADCoupledTimeDerivative
    variable = w
    v = c
  [../]
[]

[Materials]
  [./pfmobility]
    type = ADGenericConstantMaterial
    prop_names  = 'M kappa_c'
    prop_values = '100 40'
  [../]

  [./free_energy]
    type = ADMathFreeEnergy
    f_name = F
    c = 'c'
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
  scheme = 'bdf2'
  solve_type = 'NEWTON'
  num_steps = 2
  dt = 1
[]

[Outputs]
  exodus = true
  file_base = SplitCahnHilliard_out
[]
