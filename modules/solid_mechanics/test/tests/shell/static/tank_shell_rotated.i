# Test for Pressure on shell elements
# An inclined cylindrical tank (length:3m) with a wall thickness of t=0.03 m and a radius of 0.5m is subjected to an internal presure of p=40 MPa.
# The lower part of the cylinder is constrained in all directions
# Theorically, assuming a thin_walled cylinder t/r <0.1, the hoop stress is sigma_t=p*r/t
# Therefore, in-plane force in the circumference of the cylinder is F=sigma_t*t= p*r=0.5*40=20 MN (independent of material properties of the shell)
# Analytical solution for the radial displacement : u=p*r^2/(E*t)=0.00167 m
# We check the axial_force_1 at the upper part of the cylinder (far from the lower boundary to avoid boundary effects)
# The numerical modeling results in axial_force_1 =19.882 MPa (0.6% error) and radial displacement u=0.00165 (1.1% error)

[Mesh]
  [gmg]
    type = FileMeshGenerator
    file = tank_shell_rotated.msh
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

[AuxVariables]
  [axial_force_1]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [axial_force_1]
    type = ShellResultantsAux
    variable = axial_force_1
    stress_resultant = axial_force_1
    thickness = 0.03
    through_thickness_order = SECOND
    execute_on = TIMESTEP_END
  []
[]

[BCs]

  [simply_support_x]
    type = DirichletBC
    variable = disp_x
    boundary = 'lower_circle'
    value = 0.0
  []
  [simply_support_y]
    type = DirichletBC
    variable = disp_y
    boundary = 'lower_circle'
    value = 0.0
  []
  [simply_support_z]
    type = DirichletBC
    variable = disp_z
    boundary = 'lower_circle'
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

  # best overall
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = ' lu       mumps'
  line_search = 'none'
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-13
  dt = 1
  dtmin = 1
  end_time = 1
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

  [load_x]
    type = ADDistributedLoadShell
    factor = 40
    variable = disp_x
    project_load_to_normal = true
    displacements = 'disp_x disp_y disp_z'
  []
  [load_y]
    type = ADDistributedLoadShell
    factor = 40
    variable = disp_y
    project_load_to_normal = true
    displacements = 'disp_x disp_y disp_z'
  []
  [load_z]
    type = ADDistributedLoadShell
    factor = 40
    variable = disp_z
    project_load_to_normal = true
    displacements = 'disp_x disp_y disp_z'
  []
[]

[Materials]
  [elasticity]
    type = ADComputeIsotropicElasticityTensorShell
    youngs_modulus = 2e5
    poissons_ratio = 0.3

    through_thickness_order = SECOND
  []
  [strain]
    type = ADComputeIncrementalShellStrain
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y'
    thickness = 0.03
    reference_first_local_direction = ' 1 0 1'
    through_thickness_order = SECOND
  []
  [stress]
    type = ADComputeShellStress
    through_thickness_order = SECOND
  []
[]

[Outputs]
  exodus = true
[]
