# Large strain/large rotation cantilever beam tese

# A 300 N point load is applied at the end of a 4 m long cantilever beam.
# Young's modulus (E) = 1e4
# Shear modulus (G) = 1e8
# shear coefficient (k) = 1.0
# Area (A) = 1.0
# Iy = Iz = 0.16

# The non-dimensionless parameter alpha = kAGL^2/EI = 1e6
# Since the value of alpha is quite high, the beam behaves like
# a thin beam where shear effects are not significant.

# Beam deflection:
# small strain+rot = 3.998 m (exact 4.0)
# large strain + small rotation = -0.05 m in x and 3.74 m in y
# large rotations + small strain = -0.92 m in x and 2.38 m in y
# large rotations + large strain = -0.954 m in x and 2.37 m in y (exact -1.0 m in x and 2.4 m in y)

# References:
# K. E. Bisshopp and D.C. Drucker, Quaterly of Applied Mathematics, Vol 3, No. 3, 1945.

[Mesh]
  type = FileMesh
  file = beam_finite_rot_test_2.e
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
  [./force_y2]
    type = UserForcingFunctionNodalKernel
    variable = disp_y
    boundary = 2
    function = force
  [../]
[]

[Functions]
  [./force]
    type = PiecewiseLinear
    x = '0.0 2.0  8.0'
    y = '0.0 300.0 300.0'
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
  petsc_options = '-snes_ksp_ew'
  petsc_options_iname = '-ksp_gmres_restart -pc_type -pc_hypre_type -pc_hypre_boomeramg_max_iter'
  petsc_options_value = '201                hypre     boomeramg     4'
  nl_max_its = 50
  nl_rel_tol = 1e-9
  nl_abs_tol = 1e-7
  l_max_its = 50
  dt = 0.05
  end_time = 2.1
[]

[Modules/TensorMechanics/LineElementMaster]
  [./all]
  add_variables = true
  displacements = 'disp_x disp_y disp_z'
  rotations = 'rot_x rot_y rot_z'
  strain_type = FINITE
  rotation_type = FINITE

  # Geometry parameters
  area = 1.0
  Iy = 0.16
  Iz = 0.16
  y_orientation = '0.0 1.0 0.0'
  [../]
[]

[Materials]
  [./elasticity]
    type = ComputeElasticityBeam
    youngs_modulus = 1e4
    poissons_ratio = -0.99995
    shear_coefficient = 1.0
    block = 1
  [../]
  [./stress]
    type = ComputeBeamResultants
    block = 1
  [../]
[]

[Postprocessors]
  [./disp_x]
    type = PointValue
    point = '4.0 0.0 0.0'
    variable = disp_x
  [../]
  [./disp_y]
    type = PointValue
    point = '4.0 0.0 0.0'
    variable = disp_y
  [../]
  [./rot_z]
    type = PointValue
    point = '4.0 0.0 0.0'
    variable = rot_z
  [../]
[]

[Outputs]
  file_base = 'euler_finite_rot_y_out'
  exodus = true
  perf_graph = true
[]
