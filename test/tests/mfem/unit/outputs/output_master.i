[Mesh]
  type = CoupledMFEMMesh
  file = gold/simple-cube-tet10.e
  dim = 3
[]

[Variables]
  [moose_diffused]
    family = LAGRANGE
    order = FIRST
  []
[]

[AuxVariables]
  [mfem_diffused]
    family = LAGRANGE
    order = FIRST
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = moose_diffused
  []
[]

[BCs]
  [bottom]
    type = DirichletBC
    variable = moose_diffused
    boundary = 'bottom'
    value = 1
  []

  [top]
    type = DirichletBC
    variable = moose_diffused
    boundary = 'top'
    value = 0
  []
[]

[Postprocessors]
  [l2_difference]
    type = ElementL2Difference
    variable = moose_diffused
    other_variable = mfem_diffused
  []
[]

[Executioner]
  type = Transient
  dt = 1.0
  start_time = 0.0
  end_time = 2.0
  nl_abs_tol = 1e-5
  l_tol = 1e-5
[]

[Outputs]
  csv = true
  exodus = true
[]

[MultiApps]
  [sub_app]
    type = FullSolveMultiApp
    positions = '0 0 0'
    input_files = 'output_mfem.i'
    execute_on = timestep_begin
    keep_full_output_history = true
  []
[]

