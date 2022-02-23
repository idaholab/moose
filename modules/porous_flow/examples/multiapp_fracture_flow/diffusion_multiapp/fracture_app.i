# Temperature is transferred between the fracture and matrix apps
[Mesh]
  [generate]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 100
    xmin = 0
    xmax = 50.0
  []
[]

[Variables]
  [frac_T]
  []
[]

[ICs]
  [frac_T]
    type = FunctionIC
    variable = frac_T
    function = 'if(x<1E-6, 2, 0)'  # delta function
  []
[]

[AuxVariables]
  [transferred_matrix_T]
  []
[]

[Kernels]
  [dot]
    type = TimeDerivative
    variable = frac_T
  []
  [fracture_diffusion]
    type = Diffusion
    variable = frac_T
  []
  [toMatrix]
    type = PorousFlowHeatMassTransfer
    variable = frac_T
    v = transferred_matrix_T
    transfer_coefficient = 0.004
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

[VectorPostprocessors]
  [final_results]
    type = LineValueSampler
    start_point = '0 0 0'
    end_point = '50 0 0'
    num_points = 11
    sort_by = x
    variable = frac_T
    outputs = final_csv
  []
[]

[Outputs]
  print_linear_residuals = false
  [final_csv]
    type = CSV
    sync_times = 100
    sync_only = true
  []
[]

[MultiApps]
  [matrix_app]
    type = TransientMultiApp
    input_files = matrix_app.i
    execute_on = TIMESTEP_END
  []
[]

[Transfers]
  [T_to_matrix]
    type = MultiAppCopyTransfer
    to_multi_app = matrix_app
    source_variable = frac_T
    variable = transferred_frac_T
  []
  [T_from_matrix]
    type = MultiAppCopyTransfer
    from_multi_app = matrix_app
    source_variable = matrix_T
    variable = transferred_matrix_T
  []
[]
