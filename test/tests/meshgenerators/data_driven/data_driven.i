[Mesh]
  [nx]
    type = TestDataDrivenGenerator
    nx = 4
  []
  [ny]
    type = TestDataDrivenGenerator
    ny = 2
  []
  [mesh]
    type = TestDataDrivenGenerator
    nx_generator = nx
    ny_generator = ny
  []
  data_driven = true
[]
