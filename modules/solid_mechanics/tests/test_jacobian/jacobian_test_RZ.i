# This test is designed to test the jacobian for a single
# element with volumetric locking correction.
# This test uses axisymmetric small strain formulations.

# To test the jacobian obtained from finite strain axisymmetric formulation, add
# formulation = NonlinearRZ in the materials block.

# The mesh contains one element whose x or r displacement is 1 at
# the left surface (x=0) and 0 at the right surface (x=1).

# Result: The hand coded jacobian matches well with the finite
# difference jacobian with a error norm of 1.8e-16 for the axisymmetric
# small strain formulation.

# For the finite strain formulation, the error norm is in the order of 1e-5.


[Problem]
  coord_type = RZ
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 1
  ny = 1
[]

[Variables]
  [./x_disp]
    order = FIRST
    family = LAGRANGE
  [../]

  [./y_disp]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[SolidMechanics]
  [./solid]
    disp_r = x_disp
    disp_z = y_disp
  [../]
[]

[BCs]

  [./left]
    type = DirichletBC
    variable = x_disp
    boundary = left
    value = 1.0
  [../]

  [./right]
    type = DirichletBC
    variable = x_disp
    boundary = right
    value = 0.0
  [../]

[]

[Materials]
  [./elastic]
    type = Elastic
    block = 0
    disp_r = x_disp
    disp_z = y_disp
    poissons_ratio = 0.3
    youngs_modulus = 1e6
  [../]
[]

[Preconditioning]
  active = 'smp'
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient #Transient

  solve_type = NEWTON
  petsc_options = '-snes_check_jacobian -snes_check_jacobian_view'

  l_max_its = 100
  nl_abs_tol = 1e-4
  nl_rel_tol = 1e-9
  l_tol = 1e-8
  start_time = 0.0
  num_steps = 1
  dt = 0.005
  dtmin = 0.005
  end_time = 0.005
[]

[Outputs]
  exodus = true
[]
