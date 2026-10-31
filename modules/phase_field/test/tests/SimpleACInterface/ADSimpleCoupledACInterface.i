# Derived From (modules/phase_field/test/tests/SimpleACInterface/SimpleCoupledACInterface.i)

#
# Test the coupled Allen-Cahn Bulk kernel
#

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 5
  ny = 5
  nz = 0
  xmin = 0
  xmax = 50
  ymin = 0
  ymax = 50
  zmin = 0
  zmax = 50
  elem_type = QUAD4

  uniform_refine = 1
[]

[Variables]
  [w]
  []
  [eta]
    order = FIRST
    family = LAGRANGE
    [InitialCondition]
      type = SmoothCircleIC
      x1 = 25.0
      y1 = 25.0
      radius = 6.0
      invalue = 1.0
      outvalue = 0.0
      int_width = 5.0
    []
  []
[]

[Kernels]
  [detadt]
    type = ADTimeDerivative
    variable = eta
  []

  [ACBulk]
    type = ADAllenCahn
    variable = eta
    f_name = F
  []

  [CoupledBulk]
    type = ADMatReaction
    variable = eta
    v = w
    reaction_rate = L
  []

  [W]
    type = ADReaction
    variable = w
  []

  [CoupledACInterface]
    type = ADSimpleCoupledACInterface
    variable = w
    v = eta
    kappa_name = 1
  []
[]

[Materials]
  [consts]
    type = ADGenericConstantMaterial
    prop_names = 'L'
    prop_values = '1'
  []

  [free_energy]
    type = ADDerivativeParsedMaterial
    property_name = F
    coupled_variables = 'eta'
    expression = 'eta^2 * (1-eta)^2'
    derivative_order = 2
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  scheme = 'bdf2'

  solve_type = 'PJFNK'

  l_max_its = 15
  l_tol = 1.0e-4

  nl_max_its = 10
  nl_rel_tol = 1.0e-11

  start_time = 0.0
  num_steps = 1
  dt = 1
[]

[Outputs]
  hide = w
  exodus = true
[]
