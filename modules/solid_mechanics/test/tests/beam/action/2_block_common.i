# Test for LineElementAction on multiple blocks by placing parameters
# common to all blocks outside of the individual action blocks

# 2 beams of length 1m are fixed at one end and a force of 1e-4 N
# is applied at the other end of the beams. Beam 1 is in block 1
# and beam 2 is in block 2. All the material properties for the two
# beams are identical. The moment of inertia of beam 2 is twice that
# of beam 1.

# Since the end displacement of a cantilever beam is inversely proportional
# to the moment of inertia, the y displacement at the end of beam 1 should be twice
# that of beam 2.

[Mesh]
  type = FileMesh
  file = 2_beam_block.e
  displacements = 'disp_x disp_y disp_z'
[]

[BCs]
  [./fixx1]
    type = DirichletBC
    variable = disp_x
    boundary = 1
    value = 0.0
  [../]
  [./fixy1]
    type = DirichletBC
    variable = disp_y
    boundary = 1
    value = 0.0
  [../]
  [./fixz1]
    type = DirichletBC
    variable = disp_z
    boundary = 1
    value = 0.0
  [../]
  [./fixr1]
    type = DirichletBC
    variable = rot_x
    boundary = 1
    value = 0.0
  [../]
  [./fixr2]
    type = DirichletBC
    variable = rot_y
    boundary = 1
    value = 0.0
  [../]
  [./fixr3]
    type = DirichletBC
    variable = rot_z
    boundary = 1
    value = 0.0
  [../]
[]

[NodalKernels]
  [./force_1]
    type = ConstantRate
    variable = disp_y
    boundary = 2
    rate = 1e-4
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
  solve_type = PJFNK
  line_search = 'none'
  nl_max_its = 15
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-8

  dt = 1
  dtmin = 1
  end_time = 2
[]

[Modules/TensorMechanics/LineElementMaster]
  # parameters common to all blocks

  add_variables = true
  displacements = 'disp_x disp_y disp_z'
  rotations = 'rot_x rot_y rot_z'

  # Geometry parameters
  area = 0.5
  y_orientation = '0.0 1.0 0.0'

  [./block_1]
    Iy = 1e-5
    Iz = 1e-5
    block = 1
  [../]
  [./block_2]
    Iy = 2e-5
    Iz = 2e-5
    block = 2
  [../]
[]

[Materials]
  [./stress]
    type = ComputeBeamResultants
    block = '1 2'
  [../]
  [./elasticity_1]
    type = ComputeElasticityBeam
    youngs_modulus = 2.0
    poissons_ratio = 0.3
    shear_coefficient = 1.0
    block = '1 2'
  [../]
[]

[Postprocessors]
  [./disp_y_1]
    type = PointValue
    point = '1.0 0.0 0.0'
    variable = disp_y
  [../]
  [./disp_y_2]
    type = PointValue
    point = '1.0 1.0 0.0'
    variable = disp_y
  [../]
[]

[Outputs]
  file_base = '2_block_out'
  exodus = true
[]
