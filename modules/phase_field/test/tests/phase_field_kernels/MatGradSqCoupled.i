#
# Test the MatGradSqCoupled kernel (which adds -L*v to the residual) for the case
# where v is a coupled variable
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
    args = w
  [../]
# MatGradSq kernel
  [./nabla_eta]
    type = MatGradSqCoupled
    variable = w
    elec = eta
    prefactor = 0.5
  [../]
[]

[Materials]
  [./mobility]
    type = DerivativeParsedMaterial
    f_name  = L
    args = 'eta w'
    function = '(1.5-eta)^2+(1.5-w)^2'
    derivative_order = 2
  [../]
  [./free_energy]
    type = DerivativeParsedMaterial
    f_name = F
    args = 'eta'
    function = 'eta^2 * (1-eta)^2'
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

  # Preconditioned JFNK (default)
  solve_type = 'PJFNK'

  l_max_its = 15
  l_tol = 1.0e-4

  nl_max_its = 10
  nl_rel_tol = 1.0e-11

  start_time = 0.0
  num_steps = 2
  dt = 0.5
[]

# Record eta
[VectorPostprocessors]
  [./eta]
    type =  LineValueSampler
    start_point = '0 25 0'
    end_point = '50 25 0'
    variable = eta
    num_points = 15
    sort_by =  id
    execute_on = timestep_end
  [../]
[]

[Outputs]
  hide = w
#  interval = 1
  exodus = true
  console = true
  [./csv]
    type = CSV
    execute_on = final
  [../]
[]
