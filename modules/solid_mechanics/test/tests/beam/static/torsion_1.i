# Torsion test with automatically calculated Ix

# A beam of length 1 m is fixed at one end and a moment  of 5 Nm
# is applied along the axis of the beam.
# G = 7.69e9
# Ix = Iy + Iz = 2e-5
# The axial twist at the free end of the beam is:
# phi = TL/GIx = 3.25e-4

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 10
  xmin = 0.0
  xmax = 1.0
  displacements = 'disp_x disp_y disp_z'
[]

[Modules/TensorMechanics/LineElementMaster]
  [./block_all]
    add_variables = true
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'

    # Geometry parameters
    area = 0.5
    Iy = 1e-5
    Iz = 1e-5

    y_orientation = '0.0 1.0 0.0'

    block = 0
  [../]
[]

[BCs]
  [./fixx1]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0.0
  [../]
  [./fixy1]
    type = DirichletBC
    variable = disp_y
    boundary = left
    value = 0.0
  [../]
  [./fixz1]
    type = DirichletBC
    variable = disp_z
    boundary = left
    value = 0.0
  [../]
  [./fixr1]
    type = DirichletBC
    variable = rot_x
    boundary = left
    value = 0.0
  [../]
  [./fixr2]
    type = DirichletBC
    variable = rot_y
    boundary = left
    value = 0.0
  [../]
  [./fixr3]
    type = DirichletBC
    variable = rot_z
    boundary = left
    value = 0.0
  [../]
[]

[NodalKernels]
  [./force_y2]
    type = ConstantRate
    variable = rot_x
    boundary = right
    rate = 5.0
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

[Materials]
  [./elasticity]
    type = ComputeElasticityBeam
    youngs_modulus = 2.0e9
    poissons_ratio = 0.3
    shear_coefficient = 1.0
    block = 0
  [../]
  [./stress]
    type = ComputeBeamResultants
    block = 0
  [../]
[]

[Postprocessors]
  [./disp_x]
    type = PointValue
    point = '1.0 0.0 0.0'
    variable = rot_x
  [../]
[]

[Outputs]
  csv = true
  exodus = true
[]
