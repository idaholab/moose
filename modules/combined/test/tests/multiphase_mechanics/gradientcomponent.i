[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [./u]
  [../]
  [./v]
    [./InitialCondition]
      type = SmoothCircleIC
      x1 = 0.5
      y1 = 0.5
      radius = 0.2
      invalue = 1
      outvalue = 0
      int_width = 0.2
    [../]
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = v
  [../]
  [./dt]
    type = TimeDerivative
    variable = v
  [../]
  [./gradientcomponent]
    type = GradientComponent
    variable = u
    v = v
    component = 0
  [../]
[]

[Executioner]
  type = Transient
  dt = 0.1
  num_steps = 2
  solve_type = 'NEWTON'
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Outputs]
  exodus = true
[]
