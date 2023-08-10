# This test uses the custom kernal "FNSFTritiumSource" to project a tritium source term onto the outboard section of a fusion blanket
# This test projects a value of 1 on the first half near the first wall and a term of 2 on the second half near the back wall. The results where 
# visually confirmed in paraview and used as a gold file for testing.

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
  [tritium]
  []
[]

[Kernels]
  [conduction]
    type = Diffusion
    variable = tritium
  []
  [source]
    type = FNSFTritiumSource
    variable = tritium
    inner_xi = '-60 -30 -15 15 30 60'
    outer_xi = '-65 -35 -20 20 35 65'
    depth = '1 1.8 1.17'
    tritium = '1 1 1 1 1 2 2 2 2 2'
  []
[]

[BCs]
  [front]
    type = DirichletBC
    variable = tritium
    boundary = 'front'
    value = 1
  []
  [back]
    type = DirichletBC
    variable = tritium
    boundary = 'back'
    value = 3
  []
[]

[Materials]
[]