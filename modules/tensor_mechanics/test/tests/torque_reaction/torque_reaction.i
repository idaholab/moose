# Scalar torque reaction

# This test computes the sum of the torques acting on a ten element 2D bar mesh
# and is intended to replicate the classical wrench problem from statics.
# A displacement in the y along the right face is applied to the bar end to create
# a shear force along the bar end. The rotation origin default (the global origin)
# and the axis of rotation direction vector used to compute the torque reaction
# is set to (0, 0, 1) out of the plane.
# Torque is calculated for the two nodes on the left of the bar. For the bottom
# node on the right, the torque/ moment lever is the x coordinate value, and for
# the top node on the right the torque lever is the hypotenuse of the x and y
# coordinates.  The expected sum of the torque reaction is just over 37.

[GlobalParams]
  order = FIRST
  family = LAGRANGE
  displacements = 'disp_x disp_y'
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 1
[]

[Problem]
  extra_tag_vectors = 'ref'
[]

[AuxVariables]
  [./saved_x]
  [../]
  [./saved_y]
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
[]

[Modules/TensorMechanics/Master]
  [master]
    strain = SMALL
    generate_output = 'stress_xx stress_yy'
    add_variables = true
    extra_vector_tags = 'ref'
  []
[]

[BCs]
  [./left_x]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0.0
  [../]
  [./left_y]
    type = DirichletBC
    variable = disp_y
    boundary = left
    value = 0.0
  [../]
  [./right_shear_y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = right
    function = '0.001*t'
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 207000
    poissons_ratio = 0.3
  [../]
  [./elastic_stress]
    type = ComputeLinearElasticStress
  [../]
[]

[Executioner]
  type = Transient

  line_search = 'none'

  l_max_its = 30
  nl_max_its = 20
  nl_abs_tol = 1e-12
  nl_rel_tol = 1e-10
  l_tol = 1e-8

  start_time = 0.0
  dt = 0.5

  end_time = 1
  num_steps = 2
[]

[Postprocessors]
  [./_dt]
    type = TimestepSize
  [../]
  [./torque]
    type = TorqueReaction
    boundary = right
    reaction_force_variables = 'saved_x saved_y'
    direction_vector = '0. 0. 1.'
  [../]
[]

[Outputs]
  exodus = true
[]
