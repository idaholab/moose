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
# Poisson's ratio = 0.3. This gives an analytic displacement of 0.16424 m.
# FEM results from different mesh discretizations are presented below. Only
# the 10x10 mesh is included as a test.

# As shown in the table below, the results from the MOOSE FEM analysis converge
# to the analytic solution and the convergence matches well with the results
# of Dvorkin and Bathe (1984).

# Mesh of 1/8 cylinder |  FEM/analytical disp    | FEM/analytical disp
#                      |  (MOOSE implementation) | (Reported by Dvorkin)
#----------------------|-------------------------|-------------------------
#     10 x 10          |          0.82           |        0.83
#     20 x 20          |          0.95           |        0.96
#     40 x 40          |          0.99           |         -
#     80 x 80          |          1.01           |         -

[Mesh]
  [mesh]
    type = FileMeshGenerator
    file = cyl_sym_10x10.e
  []
[]

[Variables]
  [disp_x]
    order = FIRST
    family = LAGRANGE
  []
  [disp_y]
    order = FIRST
    family = LAGRANGE
  []
  [disp_z]
    order = FIRST
    family = LAGRANGE
  []
  [rot_x]
    order = FIRST
    family = LAGRANGE
  []
  [rot_y]
    order = FIRST
    family = LAGRANGE
  []
[]

[BCs]
  [simply_support_x]
    type = DirichletBC
    variable = disp_x
    boundary = 'CD AD'
    value = 0.0
  []
  [simply_support_y]
    type = DirichletBC
    variable = disp_y
    boundary = 'CD BC'
    value = 0.0
  []
  [simply_support_z]
    type = DirichletBC
    variable = disp_z
    boundary = 'AB'
    value = 0.0
  []
  # Note that the rotational DOFs are in the local coordinate system
  # Also it isn't clear from the Dvorkin paper which DOFs should be fixed on the far
  # end (boundary CD). If it were fully constrained we would need to fix disp_z and
  # the rotations, but that makes it stiffer than the analytical solution.
  [simply_support_rot_x]
    type = DirichletBC
    variable = rot_x
    boundary = 'AB'
    value = 0.0
  []
  [simply_support_rot_y]
    type = DirichletBC
    variable = rot_y
    boundary = 'AD BC'
    value = 0.0
  []
[]

[DiracKernels]
  [point]
    type = ConstantPointSource
    variable = disp_x
    point = '1 0 1'
    value = -2.5 # P = 10
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  petsc_options = '-snes_ksp_ew'
  petsc_options_iname = '-pc_type'
  petsc_options_value = ' lu'
  line_search = 'none'
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-8
  dt = 1.0
  dtmin = 1.0
  end_time = 1.0
[]

[Kernels]
  [solid_disp_x]
    type = ADStressDivergenceShell
    block = '100'
    component = 0
    variable = disp_x
    through_thickness_order = SECOND
  []
  [solid_disp_y]
    type = ADStressDivergenceShell
    block = '100'
    component = 1
    variable = disp_y
    through_thickness_order = SECOND
  []
  [solid_disp_z]
    type = ADStressDivergenceShell
    block = '100'
    component = 2
    variable = disp_z
    through_thickness_order = SECOND
  []
  [solid_rot_x]
    type = ADStressDivergenceShell
    block = '100'
    component = 3
    variable = rot_x
    through_thickness_order = SECOND
  []
  [solid_rot_y]
    type = ADStressDivergenceShell
    block = '100'
    component = 4
    variable = rot_y
    through_thickness_order = SECOND
  []
[]

[Materials]
  [elasticity]
    type = ADComputeIsotropicElasticityTensorShell
    youngs_modulus = 1e6
    poissons_ratio = 0.3
    block = '100'
    through_thickness_order = SECOND
  []
  [strain]
    type = ADComputeIncrementalShellStrain
    block = '100'
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y'
    thickness = 0.01
    through_thickness_order = SECOND
  []
  [stress]
    type = ADComputeShellStress
    block = '100'
    through_thickness_order = SECOND
  []
[]

[Postprocessors]
  [disp_x1]
    type = PointValue
    point = '1 0 1'
    variable = disp_x
  []
  [disp_y1]
    type = PointValue
    point = '1 0 1'
    variable = disp_y
  []
  [disp_x2]
    type = PointValue
    point = '0 1 1'
    variable = disp_x
  []
  [disp_y2]
    type = PointValue
    point = '0 1 1'
    variable = disp_y
  []
[]

[Outputs]
  exodus = true
  csv = true
[]
