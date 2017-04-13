# This test uses the DisplacementAboutAxis boundary condition to twist the top
# of a cylinder while the bottom face of the cylinder remains fixed.  The
# TorqueReaction postprocessor is used to calculate the applied torque acting
# on the cylinder at the top face.  This test can be extended, with a new mesh,
# to model a crack in the center of the cylinder face under type III loading.

[GlobalParams]
  order = FIRST
  family = LAGRANGE
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  file = cylinder.e
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./disp_z]
  [../]
[]

[GlobalParams]
  volumetric_locking_correction=true
[]

[AuxVariables]
  [./stress_xx]      # stress aux variables are defined for output; this is a way to get integration point variables to the output file
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./saved_x]
  [../]
  [./saved_y]
  [../]
  [./saved_z]
  [../]
[]

[Functions]
  [./rampConstantAngle]
    type = PiecewiseLinear
    x = '0. 1.'
    y = '0. 1.'
    scale_factor = 0.1
  [../]
[]

[Kernels]
  [./TensorMechanics]
    save_in = 'saved_x saved_y saved_z'
    use_displaced_mesh = true
  [../]
[]

[AuxKernels]
  [./stress_xx]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_xx
    index_i = 0
    index_j = 0
    execute_on = timestep_end
  [../]
[]

[BCs]
  [./bottom_x]
    type = DirichletBC
    variable = disp_x
    boundary = 1
    value = 0.0
  [../]
  [./bottom_y]
    type = DirichletBC
    variable = disp_y
    boundary = 1
    value = 0.0
  [../]
  [./bottom_z]
    type = DirichletBC
    variable = disp_z
    boundary = 1
    value = 0.0
  [../]
  [./top_x]
    type = DisplacementAboutAxis
    boundary = 2
    function = rampConstantAngle
    angle_units = degrees
    axis_origin = '10. 10. 10.'
    axis_direction = '0 -1.0 1.0'
    component = 0
    variable = disp_x
  [../]
  [./top_y]
    type = DisplacementAboutAxis
    boundary = 2
    function = rampConstantAngle
    angle_units = degrees
    axis_origin = '10. 10. 10.'
    axis_direction = '0 -1.0 1.0'
    component = 1
    variable = disp_y
  [../]
  [./top_z]
    type = DisplacementAboutAxis
    boundary = 2
    function = rampConstantAngle
    angle_units = degrees
    axis_origin = '10. 10. 10.'
    axis_direction = '0 -1.0 1.0'
    component = 2
    variable = disp_z
  [../]
[] # BCs

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    block = 1
    youngs_modulus = 207000
    poissons_ratio = 0.3
  [../]
  [./strain]
    type = ComputeFiniteStrain
    block = 1
  [../]
  [./elastic_stress]
    type = ComputeFiniteStrainElasticStress
    block = 1
  [../]
[]

[Executioner]

  type = Transient
  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'

  petsc_options = '-snes_ksp_ew'
  petsc_options_iname = '-ksp_gmres_restart'
  petsc_options_value = '101'

  line_search = 'none'

  l_max_its = 50
  nl_max_its = 20
  nl_abs_tol = 1e-12
  nl_rel_tol = 1e-11
  l_tol = 1e-10

  start_time = 0.0
  dt = 0.25

  end_time = 0.5
[]

[Postprocessors]
  [./torque]
    type = TorqueReaction
    boundary = 2
    react = 'saved_x saved_y saved_z'
    axis_origin = '10. 10. 10.'
    direction_vector = '0 -1.0 1.0'
  [../]
[]

[Outputs]
  file_base = torque_reaction_cylinder_out
  exodus = true
[]
