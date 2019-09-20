################################################################################
# Inputs relevant to the subchannel solver
################################################################################

[Mesh]
  type = DetailedSubchannelMesh
  nx = 6
  ny = 6
  max_dz = 0.01
[]

[AuxVariables]
  [vz]
  []
  [P]
  []
  [h]
  []
  [T]
  []
  [rho]
  []
[]

[Outputs]
  exodus = true
[]

################################################################################
# Stuff needed to make the program execute
################################################################################

[Executioner]
  type = Steady
  nl_rel_tol = 0.9
  l_tol = 0.9
[]

[Variables]
  [dummy_var]
  []
[]

[Kernels]
  [dummy_kern]
    type = Diffusion
    variable = dummy_var
  []
[]

[BCs]
  [dummy_bc1]
    variable = dummy_var
    boundary = 'bottom'
    type = DirichletBC
    value = 0
  []
  [dummy_bc2]
    variable = dummy_var
    boundary = 'top'
    type = DirichletBC
    value = 1
  []
[]
