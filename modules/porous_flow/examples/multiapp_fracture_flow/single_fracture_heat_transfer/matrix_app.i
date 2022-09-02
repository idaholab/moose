# Matrix physics, which is just heat conduction.  Heat energy comes from the fracture App
[Mesh]
  [generate]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 20
    xmin = 0
    xmax = 100.0
    ny = 9
    ymin = -9
    ymax = 9
  []
[]

[Variables]
  [matrix_T]
    initial_condition = 40 # degC
  []
[]

[Kernels]
  [dot]
    type = CoefTimeDerivative
    variable = matrix_T
    Coefficient = 1E5
  []
  [matrix_diffusion]
    type = AnisotropicDiffusion
    variable = matrix_T
    tensor_coeff = '1 0 0 0 1 0 0 0 1'
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
  dt = 1
  end_time = 100
  nl_abs_tol = 1E-3
[]


[Outputs]
  print_linear_residuals = false
  exodus = true
  csv=true
[]

[MultiApps]
  [fracture_app]
    type = TransientMultiApp
    input_files = fracture_app.i
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
