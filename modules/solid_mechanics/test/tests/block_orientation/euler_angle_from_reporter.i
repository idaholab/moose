[Mesh]
  [box]
    type = GeneratedMeshGenerator
    dim = 3
    xmax = 1
    ymax = 2
    zmax = 2
    nx = 2
    ny = 4
    nz = 4
    elem_type = HEX8
  []
  [subdomain]
    input = box
    type = SubdomainPerElementGenerator
    element_ids = '0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31'
    subdomain_ids = '0 0 0 0 1 1 1 1 2 2 2 2 3 3 3 3 4 4 4 4 5 5 5 5 6 6 6 6 7 7 7 7'
  []
[]

[Variables]
  [diffused]
    order = FIRST
    family = LAGRANGE
  []
[]

# We are testing the UO from vectorpostprocessor values.
# So the kernel does not matter here.
[Kernels]
  [diff]
    type = Diffusion
    variable = diffused
  []
[]

[BCs]
  [top]
    type = DirichletBC
    variable = diffused
    boundary = 'top'
    value = 1
  []
  [others] # arbitrary user-chosen name
    type = DirichletBC
    variable = diffused
    boundary = 'bottom left right front back'
    value = 0
  []
[]

[UserObjects]
  [euler_angle_file]
    type = EulerAngleUpdateFromReporter
    file_name = grn_8_rand.tex
    execute_on = 'timestep_begin'

    euler_angle_0_name = updated_ea/ea0
    euler_angle_1_name = updated_ea/ea1
    euler_angle_2_name = updated_ea/ea2
    grain_id_name = updated_ea/subdomain_id
  []
[]

[VectorPostprocessors]
  [block_orient]
    type = BlockOrientationVectorPostprocessor
    euler_angle_provider = euler_angle_file
    sort_by = id # sort output by elem id
  []
[]

[Reporters]
  [updated_ea]
    type = ConstantReporter
    real_vector_names = 'ea0 ea1 ea2 subdomain_id'
    real_vector_values = '10 20 30 40 50 60 70 80; 11 21 31 41 51 61 71 81; 12 22 32 42 52 62 72 82; 0 1 2 3 4 5 6 7' # Dummy value
    outputs = none
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
  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type'
  petsc_options_value = ' lu '
  nl_abs_tol = 1e-10
  nl_rel_tol = 1e-10
  nl_abs_step_tol = 1e-10

  dt = 0.05
  dtmin = 0.01
  end_time = 0.1
[]

[Outputs]
  csv = true
  execute_on = 'FINAL'
[]
