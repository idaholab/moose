# The Jacobian of the GeochemistryTimeDerivative Kernel is checked
[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 2
[]

[Variables]
  [conc]
  []
[]

[Kernels]
  [dot]
    type = GeochemistryTimeDerivative
    porosity = porosity
    variable = conc
  []
[]

[AuxVariables]
  [porosity]
  []
[]

[AuxKernels]
  [porosity]
    type = FunctionAux
    function = '1.0 + x'
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

