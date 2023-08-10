[Mesh]
  [msh]
    type = FileMeshGenerator
    file = '../../FNSF_Blanket.msh'
  []
[]

[Outputs]
  exodus = true
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type --pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Variables]
  [temp]
  []
[]

[Kernels]
  [conduction]
    type = HeatConduction
    variable = temp
  []
  [heat]
    type = FNSFOBExpHeatSource
    variable = temp
    A = 0.00001
  []
[]

[BCs]
  [back]
    type = DirichletBC
    variable = temp
    boundary = 'back'
    value = 1
  []
[]

[Materials]
  [hcm]
    type = HeatConductionMaterial
    specific_heat = 1
    thermal_conductivity = 1
  []
[]