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
    type = DirichletBC
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
  [../]
[]

[Executioner]
  type = Transient
  dt = 2e-1
  end_time = 1.0
  solve_type = Newton
  nl_rel_tol = 1E-12
[]

[Outputs]
  exodus = true
[]
