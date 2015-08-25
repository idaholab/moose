#
# This test validates the phase concentration calculation for the KKS system
#

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 20
  ny = 20
  nz = 0
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  zmin = 0
  zmax = 0
  elem_type = QUAD4
[]

# We set u
[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = 'left'
    value = 0.1
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = 'right'
    value = 0.9
  [../]
[]

[Variables]
  # primary variable
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]

  # secondary variable
  [./v]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./udiff]
    type = Diffusion
    variable = u
  [../]

  [./valgebra]
    type = AlgebraDebug
    variable = v
    v = u
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
  #solve_type = 'NEWTON'
[]

#[Preconditioning]
#  [./mydebug]
#    type = FDP
#    full = true
#  [../]
#[]

[Outputs]
  execute_on = 'timestep_end'
  file_base = bug
  exodus = true
[]
