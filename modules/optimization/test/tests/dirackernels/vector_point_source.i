[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 4
    ny = 4
    nz = 4
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
  [dot]
    type = TimeDerivative
    variable = u
  []
[]

[DiracKernels]
  [vpp_point_source]
    type = VectorPointSource
    variable = u
    value = values4D/value
    coord_x = values4D/coordx
    coord_y = values4D/coordy
    coord_z = values4D/coordz
    time = values4D/time
  []
[]

[Reporters]
  [values4D]
    type = ConstantReporter
    real_vector_names = 'coordx coordy coordz time value'
    real_vector_values = '0.25 0.75 0.25 0.75 0.25 0.75 0.25 0.75
                          0.25 0.75 0.25 0.75 0.25 0.75 0.25 0.75
                          0.25 0.75 0.25 0.75 0.25 0.75 0.25 0.75;

                          0.25 0.25 0.75 0.75 0.25 0.25 0.75 0.75
                          0.25 0.25 0.75 0.75 0.25 0.25 0.75 0.75
                          0.25 0.25 0.75 0.75 0.25 0.25 0.75 0.75;

                          0.25 0.25 0.25 0.25 0.75 0.75 0.75 0.75
                          0.25 0.25 0.25 0.25 0.75 0.75 0.75 0.75
                          0.25 0.25 0.25 0.25 0.75 0.75 0.75 0.75;

                          0.10 0.10 0.10 0.10 0.10 0.10 0.10 0.10
                          0.20 0.20 0.20 0.20 0.20 0.20 0.20 0.20
                          0.30 0.30 0.30 0.30 0.30 0.30 0.30 0.30;

                          0.00 1.00 2.00 3.00 4.00 5.00 6.00 7.00
                          8.00 9.00 10.0 11.0 12.0 13.0 14.0 15.0
                          16.0 17.0 18.0 19.0 20.0 21.0 22.0 23.0'
    outputs = none
  []
[]

[VectorPostprocessors]
  [sample]
    type = PointValueSampler
    variable = u
    points = '0.25 0.25 0.25
              0.75 0.25 0.25
              0.25 0.75 0.25
              0.75 0.75 0.25
              0.25 0.25 0.75
              0.75 0.25 0.75
              0.25 0.75 0.75
              0.75 0.75 0.75'
    sort_by = id
    execute_on = 'initial timestep_end'
  []
[]

[BCs]
  [bc]
    type = DirichletBC
    variable = u
    boundary = 'left right top bottom front back'
    value = 0
  []
[]

[Executioner]
  type = Transient
  dt = 0.1
  num_steps = 3
  solve_type = 'PJFNK'
  nl_rel_tol = 1e-10
[]

[Outputs]
  csv = true
  execute_on = 'initial timestep_end'
[]
