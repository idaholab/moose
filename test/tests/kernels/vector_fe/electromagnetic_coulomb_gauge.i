# This is an MMS problem that demonstrates solution of Maxwell's equations in the
# Coulomb gauge potential form. The equations solved are:
# -\nabla^2 V = f_{V,mms}
# -\nabla^2 A - \omega^2 A + \nabla \frac{\partial V}{\partial t} = f_{A,mms}
# This tests the value and gradient of a VectorMooseVariable as well as the time
# derivative of the gradient of a standard MooseVariable
#
# This input file is subject to two tests:
# 1) An exodiff test of the physics
# 2) A Jacobian test to verify accuracy of hand-coded Jacobian routines

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 15
  ny = 15
  xmin = -1
  ymin = -1
[]

[Variables]
  [./V]
  [../]
  [./A]
    family = LAGRANGE_VEC
    order = FIRST
    scaling = 1e-10
  [../]
[]

[Kernels]
  [./diff]
    type = CoefDiffusion
    variable = V
    coef = 5
  [../]
  [./V_frc]
    type = BodyForce
    function = 'V_forcing_function'
    variable = V
  [../]
  [./A_diff]
    type = VectorCoefDiffusion
    variable = A
    coef = 5
  [../]
  [./A_coeff_reaction]
    type = VectorCoeffReaction
    variable = A
    coefficient = -.09
  [../]
  [./A_coupled_grad_td]
    type = VectorCoupledGradientTimeDerivative
    variable = A
    v = V
  [../]
  [./A_frc]
    type = VectorBodyForce
    variable = A
    function_x = 'Ax_forcing_function'
    function_y = 'Ay_forcing_function'
    function_z = '0'
  [../]
[]

[BCs]
  [./bnd_V]
    type = FunctionDirichletBC
    variable = V
    boundary = 'left right top bottom'
    function = 'V_exact_sln'
  [../]
  [./bnd_A]
    type = VectorPenaltyDirichletBC
    variable = A
    x_exact_sln = 'Ax_exact_sln'
    y_exact_sln = 'Ay_exact_sln'
    z_exact_sln = '0'
    penalty = 1e10
    boundary = 'left right top bottom'
  [../]
[]

[Functions]
  [./V_exact_sln]
    type = ParsedFunction
    expression = 'cos(0.3*t)*cos(1.1*x)*cos(1.2*y)'
  [../]
  [./Ax_exact_sln]
    type = ParsedFunction
    expression = 'cos(0.3*t)*cos(0.4*x)*cos(0.5*y)'
  [../]
  [./Ay_exact_sln]
    type = ParsedFunction
    expression = 'cos(0.3*t)*cos(0.6*x)*cos(0.7*y)'
  [../]
  [./V_forcing_function]
    type = ParsedFunction
    expression = '0.33*sin(0.3*t)*sin(1.1*x)*cos(1.2*y) + 13.25*cos(0.3*t)*cos(1.1*x)*cos(1.2*y)'
  [../]
  [./Ax_forcing_function]
    type = ParsedFunction
    expression = '0.33*sin(0.3*t)*sin(1.1*x)*cos(1.2*y) + 1.96*cos(0.3*t)*cos(0.4*x)*cos(0.5*y)'
  [../]
  [./Ay_forcing_function]
    type = ParsedFunction
    expression = '0.36*sin(0.3*t)*sin(1.2*y)*cos(1.1*x) + 4.16*cos(0.3*t)*cos(0.6*x)*cos(0.7*y)'
  [../]
[]

[Preconditioning]
  [./pre]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 10
  end_time = 3
  l_max_its = 100
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -ksp_gmres_restart'
  petsc_options_value = 'asm 100'
  petsc_options = '-ksp_converged_reason -ksp_monitor_true_residual -ksp_monitor_singular_value -snes_linesearch_monitor'
  line_search = 'bt'
[]

[Outputs]
  exodus = true
  print_linear_residuals = false
[]

[Debug]
  show_var_residual_norms = true
[]
