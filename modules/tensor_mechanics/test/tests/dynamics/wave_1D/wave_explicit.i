# Wave propogation in 1D using Explicit Euler time integration
#
# The test is for an 1D bar element of length 4m  fixed on one end
# with a sinusoidal pulse dirichlet boundary condition applied to the other end.
# The equation of motion in terms of matrices is:
#
# M*accel +  K*disp = 0
#
# Here M is the mass matrix, K is the stiffness matrix
#
# This equation is equivalent to:
#
# density*accel + Div Stress= 0
#
# The first term on the left is evaluated using the Inertial force kernel
# The last term on the left is evaluated using StressDivergenceTensors
#
# The displacement at the second, third and fourth node at t = 0.1 are
# 0.00051581216632786,2.4823187300684e-07,5.3837815572405e-11, respectively

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1
  ny = 4
  nz = 1
  xmin = 0.0
  xmax = 0.1
  ymin = 0.0
  ymax = 4.0
  zmin = 0.0
  zmax = 0.1
[]


[Variables]
  [disp_x]
  []
  [disp_y]
    [./InitialCondition]
      type = FunctionIC
      function = 0
      function_dot = pi
    [../]
  []
  [disp_z]
  []
[]

[Kernels]
  [inertia_x]
    type = InertialForceExplicit
    variable = disp_x
  []
  [inertia_y]
    type = InertialForceExplicit
    variable = disp_y
  []
  [inertia_z]
    type = InertialForceExplicit
    variable = disp_z
  []
  [TensorMechanics]
    displacements = 'disp_x disp_y disp_z'
  []
[]

[BCs]
  [top_y]
    type = DirichletBC
    variable = disp_y
    boundary = top
    value = 0.0
    extra_matrix_tags = 'secondtime'
  []
  [top_x]
    type = DirichletBC
    variable = disp_x
    boundary = top
    value = 0.0
    extra_matrix_tags = 'secondtime'
  []
  [top_z]
    type = DirichletBC
    variable = disp_z
    boundary = top
    value = 0.0
    extra_matrix_tags = 'secondtime'
  []
  [right_x]
    type = DirichletBC
    variable = disp_x
    boundary = right
    value = 0.0
    extra_matrix_tags = 'secondtime'
  []
  [right_z]
    type = DirichletBC
    variable = disp_z
    boundary = right
    value = 0.0
    extra_matrix_tags = 'secondtime'
  []
  [left_x]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0.0
    extra_matrix_tags = 'secondtime'
  []
  [left_z]
    type = DirichletBC
    variable = disp_z
    boundary = left
    value = 0.0
    extra_matrix_tags = 'secondtime'
  []
  [front_x]
    type = DirichletBC
    variable = disp_x
    boundary = front
    value = 0.0
    extra_matrix_tags = 'secondtime'
  []
  [front_z]
    type = DirichletBC
    variable = disp_z
    boundary = front
    value = 0.0
    extra_matrix_tags = 'secondtime'
  []
  [back_x]
    type = DirichletBC
    variable = disp_x
    boundary = back
    value = 0.0
    extra_matrix_tags = 'secondtime'
  []
  [back_z]
    type = DirichletBC
    variable = disp_z
    boundary = back
    value = 0.0
    extra_matrix_tags = 'secondtime'
  []
  [bottom_x]
    type = DirichletBC
    variable = disp_x
    boundary = bottom
    value = 0.0
    extra_matrix_tags = 'secondtime'
  []
  [bottom_z]
    type = DirichletBC
    variable = disp_z
    boundary = bottom
    value = 0.0
    extra_matrix_tags = 'secondtime'
  []
  [bottom_y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = bottom
    function = 'if(t<1,sin(pi*t),0)'
    extra_matrix_tags = 'secondtime'
    implicit = false
  []
[]

[Materials]
  [Elasticity_tensor]
    type = ComputeElasticityTensor
    fill_method = symmetric_isotropic
    C_ijkl = '1 0'
  []
  [strain]
    type = ComputeSmallStrain
    displacements = 'disp_x disp_y disp_z'
  []
  [stress]
    type = ComputeLinearElasticStress
  []
  [density]
    type = GenericConstantMaterial
    prop_names = 'density'
    prop_values = '1'
  []
[]

[Executioner]
  type = Transient
  start_time = 0
  end_time = 6.0
  l_tol = 1e-12
  nl_rel_tol = 1e-12
  dt = 1e-2

  [TimeIntegrator]
    type = ActuallyExplicitEuler
    solve_type = lumped
  []
[]

[Postprocessors]
  [disp_1]
    type = NodalVariableValue
    nodeid = 1
    variable = disp_y
  []
  [disp_2]
    type = NodalVariableValue
    nodeid = 3
    variable = disp_y
  []
  [disp_3]
    type = NodalVariableValue
    nodeid = 10
    variable = disp_y
  []
  [disp_4]
    type = NodalVariableValue
    nodeid = 14
    variable = disp_y
  []
[]

[Outputs]
  interval = 10
  file_base = 'wave_explicit_out'
  exodus = true
  perf_graph = true
[]
