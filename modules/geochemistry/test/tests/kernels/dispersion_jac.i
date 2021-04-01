# Tests that the GeochemistryDispersion Jacobian is correctly computed
[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 3
  ny = 2
[]

[Variables]
  [conc]
  []
[]

[Kernels]
  [disp]
    type = GeochemistryDispersion
    variable = conc
    porosity = porosity
    tensor_coeff = '1 2 3 4 5 6 7 8 9'
  []
[]

[AuxVariables]
  [porosity]
  []
[]

[AuxKernels]
  [porosity]
    type = FunctionAux
    function = '1.0 + x + y + z'
    variable = porosity
  []
[]

[Preconditioning]
  [check]
    type = SMP
    full = true
    petsc_options = '-snes_test_jacobian -snes_force_iteration'
    petsc_options_iname = '-snes_type -ksp_type -pc_type -snes_convergence_test'
    petsc_options_value = ' ksponly    preonly   none     skip'
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  num_steps = 1
[]
