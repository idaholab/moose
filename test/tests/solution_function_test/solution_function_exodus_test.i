[Mesh]
    file = cubesource.e 
[]

[Variables]
  active = 'u'

  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Functions]
  active = 'source'
  [./source]
    type = SolutionFunction
    file_type = exodusII 
    mesh = cubesource.e 
    variable = Source 
    system = AuxSystem
  [../]
[]

[Kernels]
  active = 'diff fvol'

  [./diff]
    type = Diffusion
    variable = u
  [../]
  [./fvol]
     type = UserForcingFunction
     variable = u
     function = source
  [../]
[]

[BCs]
  active = 'stuff'

  [./stuff]
    type = DirichletBC
    variable = u
    boundary = '1'
    value = 0
  [../]

[]

[Executioner]
  type = Steady
 petsc_options = '-snes'
  l_max_its = 800
  nl_rel_tol = 1e-10
#   petsc_options = '-snes_mf_operator'
#  nl_rel_tol = 1e-10
[]

[Output]
  exodus = true
  perf_log = true
[]


