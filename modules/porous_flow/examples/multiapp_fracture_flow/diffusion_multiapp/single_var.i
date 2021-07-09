# No heat transfer between matrix and fracture, with the matrix and fracture being identical spatial domains
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
  [T]
  []
[]

[ICs]
  [T]
    type = FunctionIC
    variable = T
    function = 'if(x<0.5, 2, 0)'  # delta function
  []
[]

[Kernels]
  [dot]
    type = TimeDerivative
    variable = T
  []
  [fracture_diffusion]
    type = Diffusion
    variable = T
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
    variable = T
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
