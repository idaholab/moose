# A fracture, which is a 1D line of elements, is embedded in a matrix, which is a 2D surface of elements.
# The meshes conform: all fracture nodes are also matrix nodes (the fracture elements are sides of matrix elements).
#
# The heat equation governs temperature in the fracture and matrix system, and heat energy is transferred between the two using a MultiApp approach
[Mesh]
  [generate]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 20
    xmin = 0
    xmax = 10.0
    ny = 20
    ymin = -1.9
    ymax = 2.1
  []
[]

[Variables]
  [matrix_T]
  []
[]

[Kernels]
  [dot]
    type = TimeDerivative
    variable = matrix_T
  []
  [matrix_diffusion]
    type = AnisotropicDiffusion
    variable = matrix_T
    tensor_coeff = '1E-3 0 0 0 1E-3 0 0 0 1E-3'
  []
[]

[DiracKernels]
  [heat_from_fracture]
    type = ReporterPointSource
    variable = matrix_T
    value_name = heat_transfer_rate/transferred_joules_per_s
    x_coord_name = heat_transfer_rate/x
    y_coord_name = heat_transfer_rate/y
    z_coord_name = heat_transfer_rate/z
  []
[]

[VectorPostprocessors]
  [heat_transfer_rate]
    type = ConstantVectorPostprocessor
    vector_names = 'transferred_joules_per_s x y z'
    value = '0; 0; 0; 0'
    outputs = none
  []
[]

[Preconditioning]
  [entire_jacobian]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  dt = 100
  end_time = 100
[]


[Outputs]
  print_linear_residuals = false
  exodus = false
[]

[MultiApps]
  [fracture_app]
    type = TransientMultiApp
    input_files = fracture_app_dirac.i
    cli_args = 'Kernels/toMatrix/transfer_coefficient=0.01'
    execute_on = TIMESTEP_BEGIN
  []
[]

[Transfers]
  [T_to_fracture]
    type = MultiAppGeometricInterpolationTransfer
    to_multi_app = fracture_app
    source_variable = matrix_T
    variable = transferred_matrix_T
  []
  [heat_from_fracture]
    type = MultiAppReporterTransfer
    from_multi_app = fracture_app
    from_reporters = 'heat_transfer_rate/joules_per_s heat_transfer_rate/x heat_transfer_rate/y heat_transfer_rate/z'
    to_reporters = 'heat_transfer_rate/transferred_joules_per_s heat_transfer_rate/x heat_transfer_rate/y heat_transfer_rate/z'
  []
[]
