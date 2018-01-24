# Models fluid advecting down a single fracture sitting at x=0, and 0<=y<=3.
#
[Mesh]
  type = FileMesh
  file = 'single.e'
[]

[Variables]
  [./u]
  [../]
[]

[ICs]
  [./u_init]
    type = ConstantIC
    variable = u
    value = 0
  [../]
[]

[BCs]
  [./inj]
    type = PresetBC
    boundary = 1
    variable = u
    value = 1
  [../]
[]

[Kernels]
  [./matrix_dt]
    type = CoefTimeDerivative
    variable = u
    Coefficient = 0.2  # matrix porosity
    block = 1
  [../]
  [./matrix_diff]
    type = AnisotropicDiffusion
    variable = u
    block = 1
    tensor_coeff = '0.002 0 0   0 0 0   0 0 0'  # matrix porosity * matrix diffusivity
  [../]
  [./fracture_dt]
    type = CoefTimeDerivative
    variable = u
    Coefficient = 0.1  # fracture half-aperture * fracture porosity
    block = 2
  [../]
  [./fracture_advect]
    type = Convection
    variable = u
    block = 2
    velocity = '0 0.08 0' # fracture half-aperture * velocity in fracture
  [../]
[]

[Preconditioning]
  [./standard]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -snes_atol -snes_rtol -snes_max_it'
    petsc_options_value = 'bcgs bjacobi 1E-13 1E-20 10000'
  [../]
[]

[Executioner]
  type = Transient
  dt = 2e-1
  end_time = 1.0
  solve_type = Newton
[]

[Outputs]
  exodus = true
[]
