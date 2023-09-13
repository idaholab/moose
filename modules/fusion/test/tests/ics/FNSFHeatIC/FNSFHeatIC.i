# This test uses the custom initial condition "FNSFHeatIC" which projects an initial condition onto the outboard fusion blanket. This test projects an intial
# condtion of 1 on half of the blanket near the frist wall and a initial condition of 2 alongside the back wall. The results where visually confirmed in paraview
# and used as a gold fild for testing.

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
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Variables]
  [temp]
  []
[]

[ICs]
  [Heat]
    type = FNSFHeatIC
    variable = temp
    inner_xi = '-60 -30 -15 15 30 60'
    outer_xi = '-65 -35 -20 20 35 65'
    depth = '1 1.8 1.17'
    heat = '1 1 1 1 1 2 2 2 2 2'
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
    value = 0
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
