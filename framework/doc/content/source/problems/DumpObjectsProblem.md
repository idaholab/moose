# DumpObjectsProblem

!syntax description /Problem/DumpObjectsProblem

Run any input file overriding the `Problem/type` parameter to `DumpObjectsAction` and
setting the `Problem/dump_path` parameter to the full _hit_ input file syntax path of an
action to dump the individual Moose objects and variables created by the action.

After the parse and setup stage the `DumpObjectsProblem` will not execute the input any
further. Any objects created and parameters set by the selected action will be dumped to
the screen and Moose will halt execution.

## Example

The input file `two_block_new.i` is a test for the TensorMechanics master action, an action
that sets up (aux)variables, (aux)kernels, and materials for mechanics probems. Let's
see if we can examine what exactly a particular action block (`[./block2]`) in this file sets
up.

Compile the tensor_mechanics module executable and run

```
./tensor_mechanics-opt -i test/tests/action/two_block_new.i Problem/type=DumpObjectsProblem Problem/dump_path=Modules/TensorMechanics/Master/block2
```

You should obtain the output

```
[AuxKernels]
  [./stress_xx_block2]
    type = RankTwoAux
    block = 2
    execute_on = TIMESTEP_END
    index_i = 0
    index_j = 0
    rank_two_tensor = stress
    variable = stress_xx
  [../]
  [./strain_yy_block2]
    type = RankTwoAux
    block = 2
    execute_on = TIMESTEP_END
    index_i = 1
    index_j = 1
    rank_two_tensor = total_strain
    variable = strain_yy
  [../]
[]

[AuxVariables]
  [./stress_xx]
    blocks = '1 2'
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./strain_yy]
    blocks = '1 2'
    family = MONOMIAL
    order = CONSTANT
  [../]
[]

[Kernels]
  [./TM_block20]
    type = StressDivergenceTensors
    block = 2
    component = 0
    displacements = 'disp_x disp_y'
    use_displaced_mesh = true
    variable = disp_x
  [../]
  [./TM_block21]
    type = StressDivergenceTensors
    block = 2
    component = 1
    displacements = 'disp_x disp_y'
    use_displaced_mesh = true
    variable = disp_y
  [../]
[]

[Materials]
  [./block2_strain]
    type = ComputeFiniteStrain
    block = 2
    displacements = 'disp_x disp_y'
  [../]
[]

[Variables]
  [./disp_x]
    blocks = '1 2'
  [../]
  [./disp_y]
    blocks = '1 2'
  [../]
[]
```

which is what the

!listing test/tests/action/two_block_new.i block=Modules/TensorMechanics/Master

block in this input file creates.

The AuxVariables and AuxKernels are triggered by the `generate_outputs` parameter, the Kernels
are informed by the choice of coordinate system, as is the finite strain calculator material.

Note that this particular action creates Moose objects only for the selected blocks, while it sets up
Moose variables for the set union of all blocks handled by the action.

!syntax parameters /Problem/DumpObjectsProblem

!syntax inputs /Problem/DumpObjectsProblem

!syntax children /Problem/DumpObjectsProblem

!bibtex bibliography
