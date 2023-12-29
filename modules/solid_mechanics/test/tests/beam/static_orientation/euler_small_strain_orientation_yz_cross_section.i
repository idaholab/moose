# Test for small strain Euler beam bending in y direction

# A unit load is applied at the end of a cantilever beam of length 4m.
# The properties of the cantilever beam are as follows:
# Young's modulus (E) = 2.60072400269
# Shear modulus (G) = 1.0e4
# Poissons ratio (nu) = -0.9998699638
# Shear coefficient (k) = 0.85
# Cross-section area (A) = 0.554256
# Iy = 0.0141889 = Iz
# Length = 4 m

# For this beam, the dimensionless parameter alpha = kAGL^2/EI = 2.04e6

# The small deformation analytical deflection of the beam is given by
# delta = PL^3/3EI * (1 + 3.0 / alpha) = PL^3/3EI = 578 m

# Using 10 elements to discretize the beam element, the FEM solution is 576.866 m.
# The ratio beam FEM solution and analytical solution is 0.998.

# Beam is on the global YZ plane at a 45 deg. angle. The cross section geometry
# is non-symmetric

# References:
# Prathap and Bashyam (1982), International journal for numerical methods in engineering, vol. 18, 195-210.

[Mesh]
  type = FileMesh
  file = euler_small_strain_orientation_inclined_yz.e
  displacements = 'disp_x disp_y disp_z'
[]

[Modules/TensorMechanics/LineElementMaster]
  [./all]
    add_variables = true
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'

    # Geometry parameters
    area = 0.554256
    Ay = 0.0
    Az = 0.0
    Iy = 0.0141889
    Iz = 0.0047296333
    y_orientation = '-1.0 0 0.0'
  [../]
[]

[Materials]
  [./elasticity]
    type = ComputeElasticityBeam
    youngs_modulus = 2.60072400269
    poissons_ratio = -0.9998699638
    shear_coefficient = 0.85
    block = 0
  [../]
  [./stress]
    type = ComputeBeamResultants
    block = 0
  [../]
[]

[BCs]
  [./fixx1]
    type = DirichletBC
    variable = disp_x
    boundary = 0
    value = 0.0
  [../]
  [./fixy1]
    type = DirichletBC
    variable = disp_y
    boundary = 0
    value = 0.0
  [../]
  [./fixz1]
    type = DirichletBC
    variable = disp_z
    boundary = 0
    value = 0.0
  [../]
  [./fixr1]
    type = DirichletBC
    variable = rot_x
    boundary = 0
    value = 0.0
  [../]
  [./fixr2]
    type = DirichletBC
    variable = rot_y
    boundary = 0
    value = 0.0
  [../]
  [./fixr3]
    type = DirichletBC
    variable = rot_z
    boundary = 0
    value = 0.0
  [../]
[]

[NodalKernels]
  [./force_x2]
    type = ConstantRate
    variable = disp_x
    boundary = 1
    rate = 1.0e-4
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
  solve_type = NEWTON
  line_search = 'none'
  nl_max_its = 15
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-10

  dt = 1
  dtmin = 1
  end_time = 2
[]

[Postprocessors]
  [./disp_x]
    type = PointValue
    point = '0.0 2.8284271  2.8284271'
    variable = disp_x
  [../]
#  [./disp_y]
#    type = PointValue
#    point = '2.8284271 2.8284271 0.0'
#    variable = disp_y
#  [../]
[]

[Outputs]
  csv = true
  exodus = false
[]
