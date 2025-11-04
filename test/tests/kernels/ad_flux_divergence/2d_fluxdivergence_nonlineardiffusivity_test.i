[Mesh]
    type = GeneratedMesh
    dim = 2
    nx = 2
    ny = 2
    xmax = 1.0
    ymax = 1.0
    elem_type = QUAD4
[]

[Variables]
    [u]
    []
[]

[Kernels]
    [cres]
        type = ADFluxDivergence
        variable = u
    []

    [ctime]
        type = ADTimeDerivative
        variable = u
    []
[]

[Materials]
    [Dc]
        type = ADParsedMaterial
        property_name = diffusivity
        expression = '0.01 + u^2'
        coupled_variables = 'u'
    []

    [flux]
        type = ADFluxFromGradientMaterial
        flux = flux
        u = u
        diffusivity = diffusivity
    []
[]

[BCs]
    [left]
        type = DirichletBC
        variable = u
        boundary = 1
        value = 0
    []

    [right]
        type = NeumannBC
        variable = u
        boundary = 2
        value = 1
    []
[]

[Executioner]
    type = Transient
    solve_type = 'NEWTON'
    scheme = 'BDF2'
    dt = 1
    num_steps = 2
[]

[Outputs]
    file_base = nonlinear_diffusivity_out
    exodus = true
[]
