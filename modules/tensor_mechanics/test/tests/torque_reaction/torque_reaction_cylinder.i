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

[Problem]
  extra_tag_vectors = 'ref'
[]

[GlobalParams]
  volumetric_locking_correction=true
[]

[AuxVariables]
  [./saved_x]
  [../]
  [./saved_y]
  [../]
  [./saved_z]
  [../]
[]

[AuxKernels]
  [saved_x]
    type = TagVectorAux
    vector_tag = 'ref'
    v = 'disp_x'
    variable = 'saved_x'
  []
  [saved_y]
    type = TagVectorAux
    vector_tag = 'ref'
    v = 'disp_y'
    variable = 'saved_y'
  []
  [saved_z]
    type = TagVectorAux
    vector_tag = 'ref'
    v = 'disp_z'
    variable = 'saved_z'
  []
[]

[Modules/TensorMechanics/Master]
  [master]
    strain = FINITE
    generate_output = 'stress_xx'
    add_variables = true
    extra_vector_tags = 'ref'
  []
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
    function = '0.1*t'
    angle_units = degrees
    axis_origin = '10. 10. 10.'
    axis_direction = '0 -1.0 1.0'
    component = 0
    variable = disp_x
  [../]
  [./top_y]
    type = DisplacementAboutAxis
    boundary = 2
    function = '0.1*t'
    angle_units = degrees
    axis_origin = '10. 10. 10.'
    axis_direction = '0 -1.0 1.0'
    component = 1
    variable = disp_y
  [../]
  [./top_z]
    type = DisplacementAboutAxis
    boundary = 2
    function = '0.1*t'
    angle_units = degrees
    axis_origin = '10. 10. 10.'
    axis_direction = '0 -1.0 1.0'
    component = 2
    variable = disp_z
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 207000
    poissons_ratio = 0.3
  [../]
  [./elastic_stress]
    type = ComputeFiniteStrainElasticStress
  [../]
[]

[Executioner]

  type = Transient
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
    reaction_force_variables = 'saved_x saved_y saved_z'
    axis_origin = '10. 10. 10.'
    direction_vector = '0 -1.0 1.0'
  [../]
[]

[Outputs]
  exodus = true
[]
