[Mesh]
  #togenerate teh mesh used here execute the follwoing mesh generators
  [msh]
    type = CartesianMeshGenerator
    dim = 3
    dx = '1.5 2.4 0.1'
    dy = '1.3 0.9'
    dz = '0.4 0.5 0.6 0.7'
    ix = '2 1 1'
    iy = '2 3'
    iz = '1 1 1 1'
    subdomain_id = '0 1 1
                    2 2 2

                    3 4 4
                    5 5 5

                    0 1 1
                    2 2 2

                    3 4 4
                    5 5 5
                    '
  []
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [diff]
    type = AnisotropicDiffusion
    variable = u
    tensor_coeff = '2 0 0
                    0 4 0
        0 0 6'
  []
  [./time]
    type = TimeDerivative
    variable = u
  [../]
[]

[Functions]
  [./top_bc]
    type = ParsedFunction
    value = 'x'
  [../]
[]

[BCs]
  [./lower_left]
    type = DirichletBC
    variable = u
    boundary = 'bottom left'
    value = 1
  [../]

  [./top]
    type = FunctionNeumannBC
    variable = u
    boundary = top
    function = top_bc
  [../]

  [./right]
    type = NeumannBC
    variable = u
    boundary = right
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 2
  dt = 0.1
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
