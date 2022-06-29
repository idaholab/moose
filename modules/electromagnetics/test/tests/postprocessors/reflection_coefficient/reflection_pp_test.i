[Mesh]
  [slab]
    type = GeneratedMeshGenerator
    dim = 2
  []
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  []
[]

[Postprocessors]
  [reflection_coefficient]
    type = ReflectionCoefficient
    k = 1
    length = 1
    theta = 0
    incoming_field_magnitude = 1
    field_real = u
    field_imag = 0
    boundary = right
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Outputs]
  exodus = false
  print_linear_residuals = true
[]
