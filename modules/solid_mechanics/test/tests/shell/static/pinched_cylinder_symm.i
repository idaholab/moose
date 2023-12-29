# Test for displacement of pinched cylinder
# Ref: Figure 10 and Table 6 from Dvorkin and Bathe, Eng. Comput., Vol. 1, 1984.

# A cylinder of radius 1 m and length 2 m (along Z axis) with clamped ends
# (at z = 0 and 2 m) is pinched at mid-length by placing point loads of 10 N
# at (1, 0, 1) and (-1, 0, 1). Due to the symmetry of the problem, only 1/8th
# of the cylinder needs to be modeled.

# The normalized series solution for the displacement at the loading point is
# w = Wc E t / P = 164.24; where Wc is the displacement in m, E is the Young's
# modulus, t is the thickness and P is the point load.

# For this problem, E = 1e6 Pa, L = 2 m, R = 1 m, t = 0.01 m, P = 10 N and
# Poisson's ratio = 0.3. FEM results from different mesh discretizations are
# presented below. Only the 10x10 mesh is included as a test.

# Mesh of 1/8 cylinder |  FEM/analytical (Moose) | FEM/analytical (Dvorkin)
#                      |ratio of normalized disp.| ratio of normalized disp.
#----------------------|-------------------------|-------------------------
#     10 x 10          |          0.806          |        0.83
#     20 x 20          |          1.06           |        0.96
#     40 x 40          |          0.95           |         -
#     80 x 160         |          0.96           |         -

# The results from FEM analysis matches well with the series solution and with
# the solution presented by Dvorkin and Bathe (1984).

[Mesh]
  [./mesh]
    type = FileMeshGenerator
    file = pinched_cyl_10_10.msh
  [../]
  [./block_100]
    type = ParsedSubdomainMeshGenerator
    input = mesh
    combinatorial_geometry = 'x > -1.1 & x < 1.1 & y > -1.1 & y < 1.1 & z > -0.1 & z < 2.1'
    block_id = 100
  [../]
  [./nodeset_1]
    type = BoundingBoxNodeSetGenerator
    input = block_100
    top_right = '1.1 1.1 0'
    bottom_left = '-1.1 -1.1 0'
    new_boundary = 'CD' #CD
  [../]
  [./nodeset_2]
    type = BoundingBoxNodeSetGenerator
    input = nodeset_1
    top_right = '1.1 1.1 1.0'
    bottom_left = '-1.1 -1.1 1.0'
    new_boundary = 'AB' #AB
  [../]
  [./nodeset_3]
    type = BoundingBoxNodeSetGenerator
    input = nodeset_2
    top_right = '0.02 1.1 1.0'
    bottom_left = '-0.1 0.98 0.0'
    new_boundary = 'AD' #AD
  [../]
  [./nodeset_4]
    type = BoundingBoxNodeSetGenerator
    input = nodeset_3
    top_right = '1.1 0.02 1.0'
    bottom_left = '0.98 -0.1 0.0'
    new_boundary = 'BC' #BC
  [../]
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
[]

[BCs]
  [./simply_support_x]
    type = DirichletBC
    variable = disp_x
    boundary = 'CD AD'
    value = 0.0
  [../]
  [./simply_support_y]
    type = DirichletBC
    variable = disp_y
    boundary = 'CD BC'
    value = 0.0
  [../]
  [./simply_support_z]
    type = DirichletBC
    variable = disp_z
    boundary = 'CD AB'
    value = 0.0
  [../]
  [./simply_support_rot_x]
    type = DirichletBC
    variable = rot_x
    boundary = 'CD BC'
    value = 0.0
  [../]
  [./simply_support_rot_y]
    type = DirichletBC
    variable = rot_y
    boundary = 'CD AD'
    value = 0.0
  [../]
[]

[DiracKernels]
  [./point1]
    type = ConstantPointSource
    variable = disp_x
    point = '1 0 1'
    value = -2.5 # P = 10
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
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-8
  dt = 1.0
  dtmin = 1.0
  end_time = 1.0
[]

[Kernels]
  [./solid_disp_x]
    type = ADStressDivergenceShell
    block = '100'
    component = 0
    variable = disp_x
    through_thickness_order = SECOND
  [../]
  [./solid_disp_y]
    type = ADStressDivergenceShell
    block = '100'
    component = 1
    variable = disp_y
    through_thickness_order = SECOND
  [../]
  [./solid_disp_z]
    type = ADStressDivergenceShell
    block = '100'
    component = 2
    variable = disp_z
    through_thickness_order = SECOND
  [../]
  [./solid_rot_x]
    type = ADStressDivergenceShell
    block = '100'
    component = 3
    variable = rot_x
    through_thickness_order = SECOND
  [../]
  [./solid_rot_y]
    type = ADStressDivergenceShell
    block = '100'
    component = 4
    variable = rot_y
    through_thickness_order = SECOND
  [../]
[]

[Materials]
  [./elasticity]
    type = ADComputeIsotropicElasticityTensorShell
    youngs_modulus = 1e6
    poissons_ratio = 0.3
    block = '100'
    through_thickness_order = SECOND
  [../]
  [./strain]
    type = ADComputeIncrementalShellStrain
    block = '100'
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y'
    thickness = 0.01
    through_thickness_order = SECOND
  [../]
  [./stress]
    type = ADComputeShellStress
    block = '100'
    through_thickness_order = SECOND
  [../]
[]

[Postprocessors]
  [./disp_z2]
    type = PointValue
    point = '1 0 1'
    variable = disp_x
  [../]
[]

[Outputs]
  exodus = true
[]
