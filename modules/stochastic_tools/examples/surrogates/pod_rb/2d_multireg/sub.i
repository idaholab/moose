halfa = 10
fulla = 20

[Problem]
  type = FEProblem
  extra_tag_vectors  = 'diff0 diff1 diff2 diff3 abs0 abs1 abs2 abs3 src0 src1 src2'
[]

[Mesh]
  [msh]
    type = CartesianMeshGenerator
    dim = 2
    dx = '10 20 20 20 20 20 20 20 20'
    dy = '10 20 20 20 20 20 20 20 20'
    ix = '${halfa} ${fulla} ${fulla} ${fulla} ${fulla} ${fulla} ${fulla} ${fulla} ${fulla}'
    iy = '${halfa} ${fulla} ${fulla} ${fulla} ${fulla} ${fulla} ${fulla} ${fulla} ${fulla}'
    subdomain_id = '1 0 0 0 1 0 0 2 3
                    0 0 0 0 0 0 0 2 3
                    0 0 1 0 0 0 2 2 3
                    0 0 0 0 0 0 2 3 3
                    1 0 0 0 1 2 2 3 3
                    0 0 0 0 2 2 3 3 3
                    0 0 2 2 2 3 3 3 3
                    2 2 2 3 3 3 3 3 3
                    3 3 3 3 3 3 3 3 3'
  []
[]

[Variables]
  [psi]
  []
[]

[Kernels]
  [diff0]
    type = MatDiffusion
    variable = psi
    diffusivity = D0
    extra_vector_tags = 'diff0'
    block = 0
  []
  [diff1]
    type = MatDiffusion
    variable = psi
    diffusivity = D1
    extra_vector_tags = 'diff1'
    block = 1
  []
  [diff2]
    type = MatDiffusion
    variable = psi
    diffusivity = D2
    extra_vector_tags = 'diff2'
    block = 2
  []
  [diff3]
    type = MatDiffusion
    variable = psi
    diffusivity = D3
    extra_vector_tags = 'diff3'
    block = 3
  []
  [abs0]
    type = MaterialReaction
    variable = psi
    coefficient = absxs0
    extra_vector_tags = 'abs0'
    block = 0
  []
  [abs1]
    type = MaterialReaction
    variable = psi
    coefficient = absxs1
    extra_vector_tags = 'abs1'
    block = 1
  []
  [abs2]
    type = MaterialReaction
    variable = psi
    coefficient = absxs2
    extra_vector_tags = 'abs2'
    block = 2
  []
  [abs3]
    type = MaterialReaction
    variable = psi
    coefficient = absxs3
    extra_vector_tags = 'abs3'
    block = 3
  []
  [src0]
    type = BodyForce
    variable = psi
    value = 1
    extra_vector_tags = 'src0'
    block = 0
  []
  [src1]
    type = BodyForce
    variable = psi
    value = 1
    extra_vector_tags = 'src1'
    block = 1
  []
  [src2]
    type = BodyForce
    variable = psi
    value = 1
    extra_vector_tags = 'src2'
    block = 2
  []
[]

[Materials]
  [D0]
    type = GenericConstantMaterial
    prop_names = D0
    prop_values = 1
    block = 0
  []
  [D1]
    type = GenericConstantMaterial
    prop_names = D1
    prop_values = 1
    block = 1
  []
  [D2]
    type = GenericConstantMaterial
    prop_names = D2
    prop_values = 1
    block = 2
  []
  [D3]
    type = GenericConstantMaterial
    prop_names = D3
    prop_values = 1
    block = 3
  []
  [absxs0]
    type = GenericConstantMaterial
    prop_names = absxs0
    prop_values = 1
    block = 0
  []
  [absxs1]
    type = GenericConstantMaterial
    prop_names = absxs1
    prop_values = 1
    block = 1
  []
  [absxs2]
    type = GenericConstantMaterial
    prop_names = absxs2
    prop_values = 1
    block = 2
  []
  [absxs3]
    type = GenericConstantMaterial
    prop_names = absxs3
    prop_values = 1
    block = 3
  []
[]

[BCs]
  [left]
    type = NeumannBC
    variable = psi
    boundary = left
    value = 0
  []
  [bottom]
    type = NeumannBC
    variable = psi
    boundary = bottom
    value = 0
  []
  [top]
    type = DirichletBC
    variable = psi
    boundary = top
    value = 0
  []
  [right]
    type = DirichletBC
    variable = psi
    boundary = right
    value = 0
  []
[]

[Executioner]
  type = Steady
  solve_type = linear
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Controls]
  [stochastic]
    type = SamplerReceiver
  []
[]

[Postprocessors]
  [nodal_l2]
    type = NodalL2Norm
    variable = psi
  []
[]

[Outputs]
[]
