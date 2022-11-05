#
# Test the MatGradSquareCoupled kernel
#

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 15
  ny = 15
  nz = 0
  xmin = 0
  xmax = 50
  ymin = 0
  ymax = 50
  zmin = 0
  zmax = 50
  elem_type = QUAD4
[]

[Variables]
  [./w]
  [../]
  [./eta]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
      type = SmoothCircleIC
      x1 = 25.0
      y1 = 25.0
      radius = 6.0
      invalue = 1.0
      outvalue = 0.0
      int_width = 3.0
    [../]
  [../]
[]

[Kernels]
  [./detadt]
    type = TimeDerivative
    variable = eta
  [../]
  [./ACBulk]
    type = CoupledAllenCahn
    variable = w
    v = eta
    f_name = F
    mob_name = 1
  [../]
  [./W]
    type = MatReaction
    variable = w
    mob_name = -1
  [../]
  [./CoupledBulk]
    type = MatReaction
    variable = eta
    v = w
    mob_name = L
  [../]
  [./ACInterface]
    type = ACInterface
    variable = eta
    kappa_name = 1
    mob_name = L
    coupled_variables = w
  [../]
# MatGradSquareCoupled kernel
  [./nabla_eta]
    type = MatGradSquareCoupled
    variable = w
    elec_potential = eta
    prefactor = 0.5
  [../]
[]

[Materials]
  [./mobility]
    type = DerivativeParsedMaterial
    property_name  = L
    coupled_variables = 'eta w'
    expression = '(1.5-eta)^2+(1.5-w)^2'
    derivative_order = 2
  [../]
  [./free_energy]
    type = DerivativeParsedMaterial
    property_name = F
    coupled_variables = 'eta'
    expression = 'eta^2 * (1-eta)^2'
    derivative_order = 2
  [../]
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
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
  num_steps = 2
  dt = 0.5
[]

[Outputs]
  hide = w
  exodus = true
  console = true
[]
