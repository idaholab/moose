###########################################################
# This is a simple test with a time-dependent problem
# demonstrating the use of a central difference with a
# direct calculation of acceleration.
#
# Testing that the first and second time derivatives
# are calculated correctly using the Central Difference Direct
# method
###########################################################

[GlobalParams]
    displacements = 'disp_x disp_y'
[]

[Mesh]
    type = GeneratedMesh
    dim = 2
    xmin = -1
    xmax = 1
    ymin = -1
    ymax = 1
    nx = 1
    ny = 1
[]

[Variables]
    [disp_x]
    []
    [disp_y]
    []
[]

[Functions]
    [forcing_fn]
        type = PiecewiseLinear
        x = '0.0 0.1 0.2    0.3  0.4    0.5  0.6'
        y = '0.0 0.0 0.0025 0.01 0.0175 0.02 0.02'
    []
[]

[Kernels]
    [DynamicSolidMechanics]
        displacements = 'disp_x disp_y'
    []
    [massmatrix]
        type = MassMatrix
        density = density
        matrix_tags = 'system'
        variable = disp_x
    []
    [massmatrix_y]
        type = MassMatrix
        density = density
        matrix_tags = 'system'
        variable = disp_y
    []
[]

[Materials]
    [elasticity_tensor_block_one]
        type = ComputeIsotropicElasticityTensor
        youngs_modulus = 1e1
        poissons_ratio = 0.0
    []

    [strain_block]
        type = ComputeFiniteStrain
        displacements = 'disp_x disp_y'
        implicit = false
    []
    [stress_block]
        type = ComputeFiniteStrainElasticStress
        implicit = false
    []
    [density]
        type = GenericConstantMaterial
        prop_names = 'density'
        prop_values = 1
    []
[]

[BCs]
    [left_x]
        type = DirectFunctionDirichletBC
        variable = disp_x
        boundary = 'left'
        function = forcing_fn
    []
    [right_x]
        type = DirectFunctionDirichletBC
        variable = disp_x
        boundary = 'right'
        function = forcing_fn
    []
[]

[Executioner]
    type = Transient

    [TimeIntegrator]
        type = DirectCentralDifference
        mass_matrix_tag = 'system'
    []

    start_time = 0.0
    num_steps = 6
    dt = 0.1
[]

[Postprocessors]
    [udot]
        type = ElementAverageTimeDerivative
        variable = disp_x
    []
    [udotdot]
        type = ElementAverageSecondTimeDerivative
        variable = disp_x
    []
    [u]
        type = ElementAverageValue
        variable = disp_x
    []
[]

[Outputs]
    exodus = true
[]
