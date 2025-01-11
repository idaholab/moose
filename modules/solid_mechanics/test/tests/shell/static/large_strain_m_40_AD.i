# Large strain/rotation test for shell elements

# A cantilever beam that is 40 m long (Y direction) with 1 m x 1 m
# cross-section is modeled using 5 shell elements placed along its
# length. The bottom boundary is fixed in all displacements and
# rotations. A load of 0.140625 N is applied at each node on the top
# boundary, resulting in a total load of 0.28125 N. E = 1800 Pa and
# v = 0.0.

# The reference solution for large deflection of this beam is based on
# K. E. Bisshopp and D.C. Drucker, Quaterly of Applied Mathematics,
# Vol 3, No. # 3, 1945.

# For PL^2/EI = 3, disp_z at tip = 0.6L = 24 m & disp_y at tip = 0.76*L-L = -9.6 m

# The FEM solution at tip of cantilever is:
# disp_z = 25.2 m; relative error = 5 %
# disp_y = -9.42 m; relative error = 1.87 %

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 1
  ny = 5
  xmin = 0.0
  xmax = 1.0
  ymin = 0.0
  ymax = 40.0
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
  [fixy1]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0.0
  []
  [fixz1]
    type = DirichletBC
    variable = disp_z
    boundary = bottom
    value = 0.0
  []
  [fixr1]
    type = DirichletBC
    variable = rot_x
    boundary = bottom
    value = 0.0
  []
  [fixr2]
    type = DirichletBC
    variable = rot_y
    boundary = bottom
    value = 0.0
  []
  [fixx1]
    type = DirichletBC
    variable = disp_x
    boundary = bottom
    value = 0.0
  []
[]

[NodalKernels]
  [force_y2]
    type = UserForcingFunctionNodalKernel
    variable = disp_z
    boundary = top
    function = force_y
  []
[]

[Functions]
  [force_y]
    type = PiecewiseLinear
    x = '0.0 1.0 3.0'
    y = '0.0 1.0 1.0'
    scale_factor = 0.140625
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
  automatic_scaling = true
  petsc_options_iname = '-pc_type'
  petsc_options_value = ' lu'

  line_search = 'none'
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-14

  dt = 0.1
  dtmin = 0.1
  end_time = 1.0
[]

[Kernels]
  [solid_disp_x]
    type = ADStressDivergenceShell
    block = '0'
    component = 0
    variable = disp_x
    through_thickness_order = SECOND
    large_strain = true
  []
  [solid_disp_y]
    type = ADStressDivergenceShell
    block = '0'
    component = 1
    variable = disp_y
    through_thickness_order = SECOND
    large_strain = true
  []
  [solid_disp_z]
    type = ADStressDivergenceShell
    block = '0'
    component = 2
    variable = disp_z
    through_thickness_order = SECOND
    large_strain = true
  []
  [solid_rot_x]
    type = ADStressDivergenceShell
    block = '0'
    component = 3
    variable = rot_x
    through_thickness_order = SECOND
    large_strain = true
  []
  [solid_rot_y]
    type = ADStressDivergenceShell
    block = '0'
    component = 4
    variable = rot_y
    through_thickness_order = SECOND
    large_strain = true
  []
[]

[Materials]
  [elasticity]
    type = ADComputeIsotropicElasticityTensorShell
    youngs_modulus = 1800
    poissons_ratio = 0.0
    block = 0
    through_thickness_order = SECOND
  []
  [strain]
    type = ADComputeFiniteShellStrain
    block = '0'
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y'
    thickness = 1.0
    through_thickness_order = SECOND
  []
  [stress]
    type = ADComputeShellStress
    block = 0
    through_thickness_order = SECOND
  []
[]

[Postprocessors]
  [disp_z2]
    type = PointValue
    point = '1.0 40.0 0.0'
    variable = disp_z
  []
  [disp_y2]
    type = PointValue
    point = '1.0 40.0 0.0'
    variable = disp_y
  []
[]

[Outputs]
  exodus = true
[]
