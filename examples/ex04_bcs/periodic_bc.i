[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 50
  ny = 50
  nz = 0

  xmax = 40
  ymax = 40
  zmax = 0
  elem_type = QUAD4
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]

  [./forcing]
    type = ExampleGaussContForcing
    variable = u
    x_center = 2
    y_center = 4
  [../]

  [./dot]
    type = TimeDerivative
    variable = u
  [../]
[]

[BCs]
  [./Periodic]
    #Note: Enable either "auto" or both "manual" conditions for this example
    active = 'manual_x manual_y'

    # Can use auto_direction with Generated Meshes
    [./auto]
      variable = u
      auto_direction = 'x y'
    [../]

     # Use Translation vectors for everything else
     [./manual_x]
       variable = u
       primary = 'left'
       secondary = 'right'
       translation = '40 0 0'
     [../]

     [./manual_y]
       variable = u
       primary = 'bottom'
       secondary = 'top'
       translation = '0 40 0'
     [../]
  [../]
[]

[Executioner]
  type = Transient
  dt = 1
  num_steps = 20
  nl_rel_tol = 1e-12
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
[]
