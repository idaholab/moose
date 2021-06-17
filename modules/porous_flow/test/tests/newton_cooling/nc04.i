# Newton cooling from a bar.  Heat conduction
[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 100
  ny = 1
  xmin = 0
  xmax = 100
  ymin = 0
  ymax = 1
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'temp'
    number_fluid_phases = 0
    number_fluid_components = 0
  []
[]

[Variables]
  [temp]
  []
[]

[ICs]
  [temp]
    type = FunctionIC
    variable = temp
    function = '2-x/100'
  []
[]

[Kernels]
  [conduction]
    type = PorousFlowHeatConduction
    variable = temp
  []
[]

[Materials]
  [temperature]
    type = PorousFlowTemperature
    temperature = temp
  []
  [thermal_conductivity_irrelevant]
    type = PorousFlowThermalConductivityIdeal
    dry_thermal_conductivity = '1E2 0 0 0 1E2 0 0 0 1E2'
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = temp
    boundary = left
    value = 2
  []
  [newton]
    type = PorousFlowPiecewiseLinearSink
    variable = temp
    boundary = right
    pt_vals = '0 1 2'
    multipliers = '-1 0 1'
    flux_function = 1
  []
[]

[VectorPostprocessors]
  [temp]
    type = LineValueSampler
    variable = temp
    start_point = '0 0.5 0'
    end_point = '100 0.5 0'
    sort_by = x
    num_points = 11
    execute_on = timestep_end
  []
[]

[Preconditioning]
  [andy]
    type = SMP
    full = true
    petsc_options = '-snes_converged_reason'
    petsc_options_iname = '-ksp_type -pc_type -sub_pc_type -snes_max_it -sub_pc_factor_shift_type -pc_asm_overlap -snes_atol -snes_rtol '
    petsc_options_value = 'gmres asm lu 100 NONZERO 2 1E-14 1E-12'
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  file_base = nc04
  execute_on = timestep_end
  exodus = false
  [along_line]
    type = CSV
    execute_vector_postprocessors_on = timestep_end
  []
[]
