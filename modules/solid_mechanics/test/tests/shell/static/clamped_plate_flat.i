# Test for simply supported plate under uniform pressure

# One quarter of a 50 m x 50 m x 1m plate is modeled in this test.
# Pressure loading is applied on the top surface using nodal forces
# of magnitude -10 N on all nodes. This corresponds to a pressure (q) of
# -10.816 N/m^2.

# The FEM solution at (0,0), which is at the center of the full plate
# is -3.003319e-03 m (for a 5*5 mesh).

# The analytical solution for displacement at center of plate obtained
# using a thin plate assumption for a square plate is
# w = 16 q a^4/(D*pi^6) \sum_{m = 1,3,5, ..}^\inf \sum_{n = 1,3,5, ..}^\inf  (-1)^{(m+n-2)/2}/(mn*(m^2+n^2)^2)

# The above solution is the Naviers series solution from the "Theory of plates
# and shells" by Timoshenko and Woinowsky-Krieger (1959).

# where a = 50 m, q = -10.816 N/m^2 and D = E/(12(1-v^2))
# The analytical series solution converges to 2.998535904e-03 m
# when the first 16 terms of the series are considered (i.e., until
# m & n = 7).

[Mesh]
  [gmg]
    type = FileMeshGenerator
    file = clamped_plate_flat.msh
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
  [symm_left_rot]
    type = DirichletBC
    variable = rot_y
    boundary = 'left'
    value = 0.0
  []
  [symm_bottom_rot]
    type = DirichletBC
    variable = rot_x
    boundary = 'bottom'
    value = 0.0
  []
  [simply_support_x]
    type = DirichletBC
    variable = disp_x
    boundary = 'right top bottom left'
    value = 0.0
  []
  [simply_support_y]
    type = DirichletBC
    variable = disp_y
    boundary = 'right top bottom left'
    value = 0.0
  []
  [simply_support_z]
    type = DirichletBC
    variable = disp_z
    boundary = 'right top'
    value = 0.0
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
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = ' lu       mumps'
  line_search = 'none'
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-10
  dt = 1
  dtmin = 1
  end_time = 1.
[]

[Kernels]
  [solid_disp_x]
    type = ADStressDivergenceShell
    component = 0
    variable = disp_x
    through_thickness_order = SECOND
  []
  [solid_disp_y]
    type = ADStressDivergenceShell
    component = 1
    variable = disp_y
    through_thickness_order = SECOND
  []
  [solid_disp_z]
    type = ADStressDivergenceShell
    component = 2
    variable = disp_z
    through_thickness_order = SECOND
  []
  [solid_rot_x]
    type = ADStressDivergenceShell
    component = 3
    variable = rot_x
    through_thickness_order = SECOND
  []
  [solid_rot_y]
    type = ADStressDivergenceShell
    component = 4
    variable = rot_y
    through_thickness_order = SECOND
  []

  [load_z]
    type = ADDistributedLoadShell
    factor = 10.816
    variable = disp_z
    displacements = 'disp_x disp_y disp_z'
  []
[]

[Materials]
  [elasticity]
    type = ADComputeIsotropicElasticityTensorShell
    youngs_modulus = 1e9
    poissons_ratio = 0.3
    through_thickness_order = SECOND
    block = 'shell'
  []
  [strain]
    type = ADComputeIncrementalShellStrain
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y'
    thickness = 1
    through_thickness_order = SECOND
    block = 'shell'
  []
  [stress]
    type = ADComputeShellStress
    through_thickness_order = SECOND
    block = 'shell'
  []
[]

[Postprocessors]
  [disp_z2]
    type = PointValue
    point = '0.0 0.0 0.0'
    variable = disp_z
  []
[]

[Outputs]
  exodus = true
[]
