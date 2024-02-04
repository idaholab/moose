# Step 4a - Volumetric locking

> Keep the postprocessing modifications from the previous question, set the
> [!param](/Materials/ComputeIsotropicElasticityTensor/poissons_ratio) of the
> cantilevers to `0.49`. Then add and modify the following parameters
>
> - [!param](/Mesh/uniform_refine) in the `[Mesh]` block
> - [!param](/Modules/TensorMechanics/Master/TensorMechanicsAction/volumetric_locking_correction) in the tensor mechanics master action block
> - [!param](/Mesh/GeneratedMeshGenerator/elem_type) in the `GeneratedMeshGenerator` blocks
>
> For convenience all those parameters may be listed under `[GlobalParams]`
> instead. Compare first order QUAD4 elements to second order QUAD8 elements,
> compare the cantilever deflection with and without volumetric locking
> correction (with QUAD4 elements), and compare the result for different levels
> of uniform refinement (1, 2, 3, 4).

!listing modules/tensor_mechanics/tutorials/introduction/mech_step04a.i

We can run the various combinations of first/second order elements,
active/deactivated volumetric locking correction, and levels of refinement
and plot the resulting data.

Note that you can override input file parameters from the command line! That means you can run all the cases plotted below using these commands:

```
../../tensor_mechanics-opt -i mech_step04a.i GlobalParams/elem_type=QUAD4
../../tensor_mechanics-opt -i mech_step04a.i GlobalParams/elem_type=QUAD4 GlobalParams/volumetric_locking_correction=true
mpirun -n 4 ../../tensor_mechanics-opt -i mech_step04a.i GlobalParams/elem_type=QUAD4 GlobalParams/volumetric_locking_correction=true GlobalParams/uniform_refine=1
mpirun -n 4 ../../tensor_mechanics-opt -i mech_step04a.i GlobalParams/elem_type=QUAD8
mpirun -n 8 ../../tensor_mechanics-opt -i mech_step04a.i GlobalParams/elem_type=QUAD4 GlobalParams/volumetric_locking_correction=true GlobalParams/uniform_refine=4
mpirun -n 8 ../../tensor_mechanics-opt -i mech_step04a.i GlobalParams/elem_type=QUAD8 GlobalParams/uniform_refine=2
```

!plot scatter data=[{'x': [0.0, 0.5, 1.0, 1.5, 2.0, 2.5, 3.0, 3.5, 4.0, 4.5, 5.0],
                     'y': [0.0, 0.012297557746208, 0.024599992602543, 0.036907158654046, 0.049218909800504, 0.061535099761101, 0.073855582079212, 0.086180210127172, 0.098508837111067, 0.11084131607555, 0.12317749990864],
                     'name':'QUAD4'},
                    {'x': [0.0, 0.5, 1.0, 1.5, 2.0, 2.5, 3.0, 3.5, 4.0, 4.5, 5.0],
                     'y': [0.0, 0.021384512916666, 0.042781782730663, 0.064191101648472, 0.085611760585222, 0.10704304922261, 0.12848425606777, 0.1499346685122, 0.17139357289095, 0.19286025454192, 0.21433399786539],
                     'name':'QUAD4 + volumetric locking correction'},
                    {'x':  [0.0, 0.5, 1.0, 1.5, 2.0, 2.5, 3.0, 3.5, 4.0, 4.5, 5.0],
                     'y':  [0.0, 0.025980241948164, 0.051978049020957, 0.077992266340977, 0.10402173675443, 0.13006530093618, 0.15612179749739, 0.1821900630933, 0.2082689325313, 0.2343572388794, 0.26045381280635],
                     'name':'QUAD4 + volumetric locking correction + 1x refined'},
                    {'x':  [0.0, 0.5, 1.0, 1.5, 2.0, 2.5, 3.0, 3.5, 4.0, 4.5, 5.0],
                     'y':  [0.0, 0.025589093864284, 0.05119563282622, 0.076818475850263, 0.10245647963567, 0.12810849870985, 0.15377338552523, 0.17944999055589, 0.20513716239479, 0.23083374711374, 0.25653859103719],
                     'name':'QUAD8'},
                    {'x': [0.0, 0.5, 1.0, 1.5, 2.0, 2.5, 3.0, 3.5, 4.0, 4.5, 5.0],
                     'y': [0.0, 0.028423505022903, 0.056867312053567, 0.08532998978869, 0.1138101040149, 0.14230621773907, 0.17081689132304, 0.19934068261827, 0.22787614710086, 0.25642183704174, 0.28497630514449],
                     'name':'QUAD4 + volumetric locking correction + 4x refined'},
                    {'x':  [0.0, 0.5, 1.0, 1.5, 2.0, 2.5, 3.0, 3.5, 4.0, 4.5, 5.0],
                     'y':  [0.0, 0.02803496726704, 0.056089857227362, 0.084163290305322, 0.1122538841367, 0.14036025368994, 0.16848101139175, 0.19661476725264, 0.22476012899275, 0.25291570216821, 0.28108009029795],
                      'name':'QUAD8 + 2x refined'},
                     ]
              layout={'xaxis':{'title':'Time'},
                      'yaxis':{'title':'Maximum x displacement'},
                      'title':'Cantilever deflection'}

The point here is to make you aware that volumetric locking can occur in first
order elements with certain material properties. It can be alleviated through

- Refinement
- Use of higher order elements (QUAD8)
- Use of volumetric locking correction

Users are encouraged to carefully check their results through convergence
studies.
