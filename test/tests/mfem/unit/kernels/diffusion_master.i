[Mesh]
  type = CoupledMFEMMesh
  file = gold/mug.e
  dim = 3
[]

[Variables]
  [./moose_diffused]
    family = LAGRANGE
    order = FIRST
  [../]
[]

[AuxVariables]
  [./mfem_diffused]
    family = LAGRANGE
    order = FIRST
  [../]
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
  [./l2_difference]
    type = ElementL2Difference
    variable = moose_diffused
    other_variable = mfem_diffused
  [../]
[]

[Executioner]
  type = Transient
  dt = 1.0
  start_time = 0.0
  end_time = 1.0
[]

[Outputs]
  csv = true
  exodus = true
[]

[MultiApps]
  [sub_app]
    type = TransientMultiApp
    positions = '0 0 0'
    input_files = 'diffusion_mfem.i'
    execute_on = timestep_begin
  []
[]

[Transfers]
  [pull_diffused]
    type = MultiAppNearestNodeTransfer

    # Transfer from the sub-app to this app
    from_multi_app = sub_app

    # The name of the variable in the sub-app
    source_variable = mfem_diffused

    # The name of the auxiliary variable in this app
    variable = mfem_diffused
  []  
[]
