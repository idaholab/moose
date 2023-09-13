# This test uses the custom kernel "FNSFHeatSource" to project a temperature on an outboard fusion blanket. The test projects a vlaue of 1 on the first half of the fusion blanket near the
# first wall and a value of 2 near the back wall. The results where visually confirmed in paraview and used as a gold file for testing. There is also a manufactured solution for this test
# with a mms.i input file that uses the custom kernal.

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

[Kernels]
  [conduction]
    type = HeatConduction
    variable = temp
  []
  [heat]
    type = FNSFHeatSource
    variable = temp
    inner_xi = '-60 -30 -15 15 30 60'
    outer_xi = '-65 -35 -20 20 35 65'
    depth = '1 1.8 1.17'
    heat = '1 1 1 1 1 2 2 2 2 2'
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
