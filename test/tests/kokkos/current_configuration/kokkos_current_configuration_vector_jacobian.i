# Jacobian of vector diffusion, a curl-curl term, a coupled vector diffusion term, and a vector
# boundary flux posed on the current configuration, with respect to the solution and to nonlinear
# displacements

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
  xmin = 1
  xmax = 2
  displacements = 'disp_x disp_y'
[]

[Variables]
  [u]
    family = LAGRANGE_VEC
    [InitialCondition]
      type = VectorFunctionIC
      function_x = 'x * x + 3 * x * y'
      function_y = '1 + x * y * y'
      function_z = 'x + y'
    []
  []
  [w]
    family = LAGRANGE_VEC
    [InitialCondition]
      type = VectorFunctionIC
      function_x = 'x * y'
      function_y = 'x - y * y'
      function_z = '2 * x * y'
    []
  []
  [disp_x]
    [InitialCondition]
      type = FunctionIC
      function = '0.1 * x * y + 0.2 * x'
    []
  []
  [disp_y]
    [InitialCondition]
      type = FunctionIC
      function = '0.2 * x * x + 0.5 * y'
    []
  []
  [disp_z]
    [InitialCondition]
      type = FunctionIC
      function = '0.1 * x * z + 0.3 * y * z'
    []
  []
[]

[Kernels]
  [diffusion]
    type = KokkosVectorDiffusion
    variable = u
    use_displaced_mesh = true
  []
  [curl_curl]
    type = KokkosVectorCurlCurl
    variable = u
    use_displaced_mesh = true
  []
  [w_diffusion]
    type = KokkosVectorDiffusion
    variable = w
    use_displaced_mesh = true
  []
  [w_coupled]
    type = KokkosCoupledVectorDiffusion
    variable = w
    v = u
    use_displaced_mesh = true
  []
  [disp_x]
    type = KokkosDiffusion
    variable = disp_x
  []
  [disp_y]
    type = KokkosDiffusion
    variable = disp_y
  []
  [disp_z]
    type = KokkosDiffusion
    variable = disp_z
  []
[]

[BCs]
  [flux]
    type = KokkosVectorNeumannBC
    variable = u
    boundary = 'right top'
    values = '1 2 3'
    use_displaced_mesh = true
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
