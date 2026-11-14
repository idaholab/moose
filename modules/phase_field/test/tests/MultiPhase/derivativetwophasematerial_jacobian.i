#
# Jacobian test for the third derivatives of DerivativeTwoPhaseMaterial. The
# non-split CahnHilliard kernel is the only consumer of d^3F/dc^2deta and
# d^3F/dcdeta^2, so it is used here to exercise the mixed third derivatives
# with respect to one order parameter and two composition arguments.
#

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 6
  ny = 6
  xmax = 30
  ymax = 30
  elem_type = QUAD4
[]

[Variables]
  [c]
    order = FIRST
    family = LAGRANGE
    [InitialCondition]
      type = SmoothCircleIC
      x1 = 15.0
      y1 = 15.0
      radius = 6.0
      invalue = 0.9
      outvalue = 0.1
      int_width = 6.0
    []
  []
  [eta]
    order = FIRST
    family = LAGRANGE
    [InitialCondition]
      type = SmoothCircleIC
      x1 = 18.0
      y1 = 15.0
      radius = 4.0
      invalue = 0.9
      outvalue = 0.1
      int_width = 4.0
    []
  []
[]

[Kernels]
  [dcdt]
    type = TimeDerivative
    variable = c
  []
  [CHSolid]
    type = CahnHilliard
    variable = c
    coupled_variables = 'eta'
    f_name = F
    mob_name = M
  []

  [detadt]
    type = TimeDerivative
    variable = eta
  []
  [ACBulk]
    type = AllenCahn
    variable = eta
    coupled_variables = 'c'
    f_name = F
  []
  [ACInterface]
    type = ACInterface
    variable = eta
    kappa_name = kappa_eta
  []
[]

[Materials]
  [consts]
    type = GenericConstantMaterial
    prop_names = 'M L kappa_eta'
    prop_values = '1 1 1'
  []

  [switching]
    type = SwitchingFunctionMaterial
    eta = eta
    h_order = SIMPLE
  []
  [barrier]
    type = BarrierFunctionMaterial
    eta = eta
    g_order = SIMPLE
  []

  [free_energy_A]
    type = DerivativeParsedMaterial
    property_name = Fa
    coupled_variables = 'c'
    expression = '(c-0.1)^2*(c-1)^2 + c*0.01'
    derivative_order = 3
  []
  [free_energy_B]
    type = DerivativeParsedMaterial
    property_name = Fb
    coupled_variables = 'c'
    expression = 'c^2*(c-0.9)^2 + (1-c)*0.01'
    derivative_order = 3
  []

  [free_energy]
    type = DerivativeTwoPhaseMaterial
    property_name = F
    fa_name = Fa
    fb_name = Fb
    coupled_variables = 'c'
    eta = eta
    W = 1
    derivative_order = 3
  []
[]

[Preconditioning]
  [SMP]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'

  num_steps = 1
  dt = 0.01
[]

[Outputs]
  exodus = false
[]
