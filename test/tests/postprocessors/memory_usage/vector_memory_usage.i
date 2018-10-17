[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 60
  ny = 60
[]

[GlobalParams]
  order = CONSTANT
  family = MONOMIAL
[]

[Variables]
  [./u]
  [../]
[]

[AuxVariables]
  [./processor_id]
  [../]
  [./hardware_id]
  [../]
  [./physical_mem]
  [../]
  [./node_utilization]
  [../]
[]

[AuxKernels]
  [./processor_id]
    type = ProcessorIDAux
    variable = processor_id
  [../]
  [./hardware_id]
    type = VectorPostprocessorVisualizationAux
    variable = hardware_id
    vpp = mem
    vector_name = hardware_id
  [../]
  [./physical_mem]
    type = VectorPostprocessorVisualizationAux
    variable = physical_mem
    vpp = mem
    vector_name = physical_mem
  [../]
  [./node_utilization]
    type = VectorPostprocessorVisualizationAux
    variable = node_utilization
    vpp = mem
    vector_name = node_utilization
  [../]
[]

[Adaptivity]
  [./Markers]
    [./box]
      type = BoxMarker
      bottom_left = '0.6 0.7 0'
      top_right = '0.9 0.9 0'
      inside = refine
      outside = do_nothing
    [../]
  [../]
  marker = box
[]

[Problem]
  kernel_coverage_check = false
  solve = false
[]

[VectorPostprocessors]
  [./mem]
    type = VectorMemoryUsage
    execute_on = 'INITIAL TIMESTEP_END NONLINEAR LINEAR'
    report_peak_value = true
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 5
[]

[Outputs]
  csv = true
  nemesis = true
[]
