[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    nx = 20
    dim = 1
  []
[]

[Variables]
  [u][]
  [v][]
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
  [diff_v]
    type = Diffusion
    variable = v
  []
  [coupled]
    type = RenamedCoupledForce
    variable = v
    coupled_force_variable = u
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 1
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 0
  []
  [left_v]
    type = DirichletBC
    variable = v
    boundary = left
    value = 1
  []
  [right_v]
    type = DirichletBC
    variable = v
    boundary = right
    value = 0
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
[]

[Postprocessors]
  [avg_u]
    type = ElementAverageValue
    variable = v
  []
[]

[Outputs]
  csv = true
[]
