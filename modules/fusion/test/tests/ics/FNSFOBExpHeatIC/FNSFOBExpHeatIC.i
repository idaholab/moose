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

[ICs]
  [Heat]
    type = FNSFOBExpHeatIC
    A = 10
    variable = temp
  []
[]

[Kernels]
  [conduction]
    type = HeatConduction
    variable = temp
  []
[]

[BCs]
  [front]
    type = DirichletBC
    variable = temp
    boundary = 'front'
    value = 1
  []
  [back]
    type = DirichletBC
    variable = temp
    boundary = 'back'
    value = 3
  []
[]

[Materials]
  [hcm]
    type = HeatConductionMaterial
    specific_heat = 1
    thermal_conductivity = 1
  []
[]