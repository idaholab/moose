# example of a traveling wave solution for the double obstacle potential
# governing equation:
#   tao * phi_dot = epsilon * laplace(phi) + gamma * (phi - 0.5) + sqrt(phi * (1 - phi)) * m;
# coefficients:
#   epsilon = 8 * sigma * eta / pi / pi, gamma = 8 * sigma / eta,
#   tao = 8 * eta / mu / pi / pi,        m = -8 / pi * delta_g
# parameters:
#   delta_g = 1.0, eta = 6.0, mu = 1.0, sigma = 1.0
# reference:
#   I. Steinbach, Modelling and Simulation in Materials Science and Engineering. 17(7) (2009) 073001.
#   website: https://iopscience.iop.org/article/10.1088/0965-0393/17/7/073001/pdf 
# analytical solution:
#   Eq.(67) in the reference

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 100
  xmin = -50
  xmax = 50
  elem_type = EDGE
[]

[Variables]
  [./phi]
    family = LAGRANGE
    order = FIRST
  [../]
[]

[ICs]
  [./ICc]
    type = FunctionIC
    variable = phi
    function = "if(x < 0, 0, 1)"
  [../]
[]

[Kernels]
  [tws_diffusion]
    type = MatDiffusion
    variable = phi
    diffusivity = epsilon
  []
  [tws_time_derivative]
    type = SusceptibilityTimeDerivative
    variable = phi
    f_name = tao
  []
  [./tws_drift]
    type = TWSDoubleObstacle
    variable = phi
    sigma = sigma
    eta = eta
    delta_g = dg
  [../]
[]

[AuxVariables]
  [./bounds_dummy]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Bounds]
  [./phi_upper_bound]
    type = ConstantBoundsAux
    variable = bounds_dummy
    bounded_variable = phi
    bound_type = upper
    bound_value = 1.0
    execute_on = 'INITIAL LINEAR NONLINEAR TIMESTEP_END TIMESTEP_BEGIN FINAL'
  [../]
  [./phi_lower_bound]
    type = ConstantBoundsAux
    variable = bounds_dummy
    bounded_variable = phi
    bound_type = lower
    bound_value = 0.0
    execute_on = 'INITIAL LINEAR NONLINEAR TIMESTEP_END TIMESTEP_BEGIN FINAL'
  [../]
[]

[BCs]
  [./left]
    type = NeumannBC
    variable = phi
    boundary = left
    value = 0
  [../]
  [./right]
    type = NeumannBC
    variable = phi
    boundary = right
    value = 0
  [../]
[]

[Materials]
  [./sim_params]
    type = ADGenericConstantMaterial
    prop_names  = 'dg  mu sigma eta pi'
    prop_values = '1.0 1.0 1.0 6.0 3.14159265'
  [../]
  [./tao]
    type = ADParsedMaterial 
    f_name = tao
    function = '8.0 * eta / mu / pi / pi'
    material_property_names = 'eta mu pi'
  [../]
  [./epsilon]
    type = ADParsedMaterial 
    f_name = epsilon
    function = '8.0 * sigma * eta / pi / pi'
    material_property_names = 'eta sigma pi'
  [../]
[]

[Executioner]
  type = Transient
  solve_type = PJFNK
  petsc_options_iname = '-pc_type  -snes_type'
  petsc_options_value = 'lu vinewtonrsls'
  end_time = 10
  dt = 0.1
[]

[Outputs]
  interval = 1
  exodus = true
[]
