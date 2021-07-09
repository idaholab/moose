# Heat transfer between matrix and fracture, with the matrix and fracture being identical spatial domains, but a multiapp approach is not used
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
  [matrix_T]
  []
[]

[ICs]
  [frac_T]
    type = FunctionIC
    variable = frac_T
    function = 'if(x<0.5, 2, 0)'  # delta function
  []
[]

[Kernels]
  [dot_frac]
    type = TimeDerivative
    variable = frac_T
  []
  [frac_diffusion]
    type = Diffusion
    variable = frac_T
  []
  [toMatrix]
    type = PorousFlowHeatMassTransfer
    variable = frac_T
    v = matrix_T
    transfer_coefficient = 0.004
  []
  [dot_matrix]
    type = TimeDerivative
    variable = matrix_T
  []
  [matrix_diffusion]
    type = Diffusion
    variable = matrix_T
  []
  [toFrac]
    type = PorousFlowHeatMassTransfer
    variable = matrix_T
    v = frac_T
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
    variable = 'frac_T matrix_T'
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
