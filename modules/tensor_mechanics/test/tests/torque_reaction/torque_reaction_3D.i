# Scalar torque reaction

# This test computes the sum of the torques acting on a single element cube mesh.
# Equal displacements in the x and the z are applied along the cube top to
# create a shear force along the (1, 0, 1) direction.  The rotation origin is
# set to the middle of the bottom face of the cube (0.5, 0, 0.5), and the axis of
# rotation direction vector  used to compute the torque reaction is set to (-1, 0, 1).
# Torque is calculated for the four nodes on the top of the cube. The projection
# of the node coordinates is zero for nodes 3 and 6, +1 for node 7, and -1 for
# node 2 from the selection of the direction vector and the rotation axis origin.

[GlobalParams]
  order = FIRST
  family = LAGRANGE
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1
  ny = 1
  nz = 1
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
    strain = SMALL
    generate_output = 'stress_xx stress_yy stress_zz'
    add_variables = true
    extra_vector_tags = 'ref'
  []
[]

[BCs]
  [./bottom_x]
    type = DirichletBC
    variable = disp_x
    boundary = bottom
    value = 0.0
  [../]
  [./bottom_y]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0.0
  [../]
  [./bottom_z]
    type = DirichletBC
    variable = disp_z
    boundary = bottom
    value = 0.0
  [../]
  [./top_shear_z]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = top
    function = '0.01*t'
  [../]
  [./top_shear_x]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = top
    function = '0.01*t'
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

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'

  petsc_options = '-snes_ksp_ew'
  petsc_options_iname = '-ksp_gmres_restart'
  petsc_options_value = '101'

  line_search = 'none'

  l_max_its = 30
  nl_max_its = 20
  nl_abs_tol = 1e-14
  nl_rel_tol = 1e-12
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
    boundary = top
    reaction_force_variables = 'saved_x saved_y saved_z'
    axis_origin = '0.5 0. 0.5'
    direction_vector = '-1. 0. 1.'
  [../]
[]

[Outputs]
  exodus = true
[]
