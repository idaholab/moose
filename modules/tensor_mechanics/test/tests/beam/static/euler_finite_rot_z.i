# Large strain/large rotation cantilever beam test

# A 300 N point load is applied at the end of a 4 m long cantilever beam.
# Young's modulus (E) = 1e4
# Shear modulus (G) = 1e8
# Poisson's ratio (nu) = -0.99995
# shear coefficient (k) = 1.0
# Area (A) = 1.0
# Iy = Iz = 0.16

# The dimensionless parameter alpha = kAGL^2/EI = 1e6
# Since the value of alpha ia quite high, the beam behaves like
# a thin beam where shear effects are not significant.

# Beam deflection:
# small strain+rot = 3.998 m (exact 4.0)
# large strain + small rotation = -0.05 m in x and 3.74 m in z
# large rotations + small strain = -0.92 m in x and 2.38 m in z
# large rotations + large strain = -0.954 m in x and 2.37 m in z (exact -1.0 m in x and 2.4 m in z)

# References:
# K. E. Bisshopp and D.C. Drucker, Quaterly of Applied Mathematics, Vol 3, No. 3, 1945.

[Mesh]
  type = FileMesh
  file = beam_finite_rot_test_2.e
  displacements = 'disp_x disp_y disp_z'
[]

[Variables]
  [./disp_x]
    order = FIRST
    family = LAGRANGE
  [../]
  [./disp_y]
    order = FIRST
    family = LAGRANGE
  [../]
  [./disp_z]
    order = FIRST
    family = LAGRANGE
  [../]
  [./rot_x]
    order = FIRST
    family = LAGRANGE
  [../]
  [./rot_y]
    order = FIRST
    family = LAGRANGE
  [../]
  [./rot_z]
    order = FIRST
    family = LAGRANGE
  [../]
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
  [./force_z2]
    type = UserForcingFunctionNodalKernel
    variable = disp_z
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

[Kernels]
  [./solid_disp_x]
    type = StressDivergenceBeam
    block = '1'
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
    component = 0
    variable = disp_x
  [../]
  [./solid_disp_y]
    type = StressDivergenceBeam
    block = '1'
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
    component = 1
    variable = disp_y
  [../]
  [./solid_disp_z]
    type = StressDivergenceBeam
    block = '1'
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
    component = 2
    variable = disp_z
  [../]
  [./solid_rot_x]
    type = StressDivergenceBeam
    block = '1'
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
    component = 3
    variable = rot_x
  [../]
  [./solid_rot_y]
    type = StressDivergenceBeam
    block = '1'
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
    component = 4
    variable = rot_y
  [../]
  [./solid_rot_z]
    type = StressDivergenceBeam
    block = '1'
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
    component = 5
    variable = rot_z
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
  [./strain]
    type = ComputeFiniteBeamStrain
    block = '1'
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
    area = 1.0
    Ay = 0.0
    Az = 0.0
    Iy = 0.16
    Iz = 0.16
    y_orientation = '0.0 1.0 0.0'
    large_strain = true
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
    variable = disp_z
  [../]
  [./rot_z]
    type = PointValue
    point = '4.0 0.0 0.0'
    variable = rot_y
  [../]
[]

[Outputs]
  exodus = true
  perf_graph = true
[]
