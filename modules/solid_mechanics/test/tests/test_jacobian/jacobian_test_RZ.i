# This test is designed to test the jacobian for a single
# element with/without  volumetric locking correction.

# Result: The hand coded jacobian matches well with the finite
# difference jacobian with an error norm in the order of 1e-15
# for total and incremental small strain and with an error norm
# in the order of 1e-8 for finite strain.


[Problem]
  coord_type = RZ
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 1
  ny = 1
  xmin = 1
  xmax = 1.75
  ymin = 0
  ymax = 1.5
[]

[GlobalParams]
  displacements = 'disp_x disp_y'
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
[]

[Modules/TensorMechanics/Master]
  [all]
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = disp_x
    preset = false
    boundary = left
    value = 1.0
  []
[]

[Materials]
  [elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1e6
    poissons_ratio = 0.3
    block = 0
  []
  [stress]
    block = 0
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient #Transient

  solve_type = NEWTON
  petsc_options = '-snes_test_jacobian -snes_test_jacobian_view'

  l_max_its = 1
  nl_abs_tol = 1e-4
  nl_rel_tol = 1e-6
  l_tol = 1e-6
  start_time = 0.0
  num_steps = 1
  dt = 0.005
  dtmin = 0.005
  end_time = 0.005
[]
