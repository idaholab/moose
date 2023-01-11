[Mesh]
  file = nonmatching.e
  displacements = 'disp_x disp_y disp_z'
[]

[Variables]
  [./temp]
  [../]
[]

[AuxVariables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./disp_z]
  [../]
[]

[Functions]
  [./disp_y]
    type = ParsedFunction
    expression = 0.1*t
  [../]
  [./left_temp]
    type = ParsedFunction
    expression = 1000+t
  [../]
[]

[Kernels]
  [./hc]
    type = HeatConduction
    variable = temp
  [../]
[]

[AuxKernels]
  [./disp_y]
    type = FunctionAux
    variable = disp_y
    function = disp_y
    block = left
  [../]
[]

[BCs]
  [./left]
    type = FunctionDirichletBC
    variable = temp
    boundary = leftleft
    function = left_temp
  [../]
  [./right]
    type = DirichletBC
    variable = temp
    boundary = rightright
    value = 400
  [../]
[]

[ThermalContact]
  [./left_to_right]
    type = GapHeatTransfer
    variable = temp
    primary = rightleft
    secondary = leftright
    emissivity_primary = 0
    emissivity_secondary = 0
    quadrature = true
  [../]
[]

[Materials]
  [./hcm]
    type = HeatConductionMaterial
    block = 'left right'
    specific_heat = 1
    thermal_conductivity = 1
    use_displaced_mesh = true
  [../]
[]

[Postprocessors]
  [./left]
    type = SideDiffusiveFluxIntegral
    variable = temp
    boundary = leftright
    diffusivity = thermal_conductivity
  [../]
  [./right]
    type = SideDiffusiveFluxIntegral
    variable = temp
    boundary = rightleft
    diffusivity = thermal_conductivity
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 9
  dt = 1

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
