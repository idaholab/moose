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
    nx = 4
    ny = 4
[]

[Variables]
    [disp_x]
    []
    [disp_y]
    []
[]

[Functions]
    [forcing_fn]
        type = ParsedFunction
        expression = 't'
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
        poissons_ratio = 0.33
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
        type = DirectDirichletBC
        variable = disp_x
        boundary = 'left'
        value = 0
    []
    [left_y]
        type = DirectDirichletBC
        variable = disp_y
        boundary = 'left'
        value = 0
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
    [left_x]
        type = AverageNodalVariableValue
        variable = disp_x
        boundary = left
    []
    [right_y]
        type = AverageNodalVariableValue
        variable = disp_x
        boundary = left
    []
[]

[Outputs]
    exodus = true
[]
