[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  [file]
    type = FileMeshGenerator
    file = 2blk-gap.e
  []
  allow_renumbering = false
[]

[Problem]
  kernel_coverage_check = false
  material_coverage_check = false
[]

[Variables]
  [temp]
    order = FIRST
    family = LAGRANGE
    block = '1 2'
  []
  [disp_x]
    order = FIRST
    family = LAGRANGE
    block = '1 2'
  []
  [disp_y]
    order = FIRST
    family = LAGRANGE
    block = '1 2'
  []
[]

[Materials]
  [left]
    type = ADHeatConductionMaterial
    block = 1
    thermal_conductivity = 0.01
    specific_heat = 1
  []

  [right]
    type = ADHeatConductionMaterial
    block = 2
    thermal_conductivity = 0.005
    specific_heat = 1
  []
[]

[Kernels]
  [hc_displaced_block]
    type = ADHeatConduction
    variable = temp
    use_displaced_mesh = true
    block = '1'
  []
  [hc_undisplaced_block]
    type = ADHeatConduction
    variable = temp
    use_displaced_mesh = false
    block = '2'
  []
  [disp_x]
    type = Diffusion
    variable = disp_x
    block = '1 2'
  []
  [disp_y]
    type = Diffusion
    variable = disp_y
    block = '1 2'
  []
[]

[MortarGapHeatTransfer]
  [mortar_heat_transfer]
   temperature = temp

   primary_emissivity = 1.0
   secondary_emissivity = 1.0
   boundary = 100
   use_displaced_mesh = true
   gap_conductivity = 0.02

   primary_boundary = 100
   secondary_boundary = 101
   gap_flux_options = 'CONDUCTION RADIATION'
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = temp
    boundary = 'left'
    value = 100
  []

  [right]
    type = DirichletBC
    variable = temp
    boundary = 'right'
    value = 0
  []

  [left_disp_x]
    type = DirichletBC
    preset = false
    variable = disp_x
    boundary = 'left'
    value = .1
  []
  [right_disp_x]
    type = DirichletBC
    preset = false
    variable = disp_x
    boundary = 'right'
    value = 0
  []
  [bottom_disp_y]
    type = DirichletBC
    preset = false
    variable = disp_y
    boundary = 'bottom'
    value = 0
  []
[]

[Preconditioning]
  [fmp]
    type = SMP
    full = true
    solve_type = 'NEWTON'
  []
[]

[Executioner]
  type = Steady
  nl_rel_tol = 1e-11
  nl_abs_tol = 1.0e-10
[]

[VectorPostprocessors]
  [NodalTemperature]
    type = NodalValueSampler
    sort_by = id
    boundary = '100 101'
    variable = 'temp'
  []
[]

[Outputs]
  csv = true
  [exodus]
    type = Exodus
    show = 'temp'
  []
[]
