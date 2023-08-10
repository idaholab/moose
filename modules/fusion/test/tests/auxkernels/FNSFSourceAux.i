# This test uses the custom auxkernel "FNSFSourceAux" to project a coupled force onto an outboard blanket. It projects a value of 1 on the first half of the blanket
# near the first wall and a value of 2 onto the second half of the blanket near the back wall. The results where visually checked in paraview and an exodiff test was
# put into place using the visually confirmed output file as the gold file.

[Mesh]
  [msh]
    type = FileMeshGenerator
    file = '../FNSF_Blanket.msh'
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
  [heat_generation]
    type = CoupledForce
    variable = temp
    v = power_density
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
    value = 2
  []
[]

[Materials]
  [hcm]
    type = HeatConductionMaterial
    specific_heat = 1
    thermal_conductivity = 1
  []
[]

[AuxVariables]
  [power_density]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [power]
    type = FNSFSourceAux
    variable = power_density
    inner_xi = '-60 -30 -15 15 30 60'
    outer_xi = '-65 -35 -20 20 35 65'
    depth = '1 1.8 1.17'
    source = '1 1 1 1 1 2 2 2 2 2'
  []
[]
