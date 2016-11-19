#Visualization of stresses and strains and other tensor components

To visualize stresses, strains, and elasticity tensor components, the material objects must be outputted to auxiliary variables using auxiliary kernels available. RankTwoAux is used to output a RankTwoTensor component, and RankFourAux is used to output a RankFourTensor component.  For example, $\sigma_{11}, \epsilon_{22}$, and $C_{1122}$ can be visualized by first declaring auxiliary variables for them in the input file:

```
    [AuxVariables]
      [./s11_aux]
        order = CONSTANT
        family = MONOMIAL
      [../]

      [./e22_aux]
        order = CONSTANT
        family = MONOMIAL
      [../]

      [./C1122_aux]
        order = CONSTANT
        family = MONOMIAL
      [../]
    []
```
 
Next, the appropriate auxiliary kernels are used.  They require the name of the material property that you wish to see the field value for, and the indices of the tensor value (either 0, 1, or 2).  For example, 

```
    [AuxKernels]
      [./report_s11]
        type = RankTwoAux
        rank_two_tensor = stress      # this is the name of the material property
        index_i = 0                   # this is the first index, i, from 0 to 2
        index_j = 0                   # this is the 2nd index, j, from 0 to 2
        variable = s11_aux            # auxilliary variable declared previously
      [../]

      [./report_e22]
        type = RankTwoAux
        rank_two_tensor = elastic_strain
        index_i = 1
        index_j = 1
        variable = e22_aux
      [../]

      [./report_C1122]
        type = RankFourAux
        rank_four_tensor = elasticity_tensor
        index_i = 0
        index_j = 0
        index_k = 1
        index_l = 1
        variable = C1122_aux
      [../]
    []
```
