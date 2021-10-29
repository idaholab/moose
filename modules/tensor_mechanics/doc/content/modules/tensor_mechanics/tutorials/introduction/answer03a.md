> Try and add output for the vonMises stress in the simulation domain. Take a
> look at the
> [!param](/Modules/TensorMechanics/Master/TensorMechanicsAction/generate_output)
> parameter...

`generate_output` is a convenient way to obtain scalar quantities, such as
tensor components, invariants, etc. as fields that can be visualized on a mesh.

Change your master action block to

```
[Modules/TensorMechanics/Master]
  [all]
    add_variables = true
    generate_output = 'vonmises_stress'
  []
[]
```

Then rerun the input and visualize the output. You will notice that the
resulting field looks blocky. MOOSE is projecting the material point values onto
constant monomial functions. The
[!param](/Modules/TensorMechanics/Master/TensorMechanicsAction/material_output_family)
and
[!param](/Modules/TensorMechanics/Master/TensorMechanicsAction/material_output_order)
parameters allow to to select higher order monomials and even nodal variables
for smooth material property output. For now let's just try a second order
monomial with

```
[Modules/TensorMechanics/Master]
  [all]
    add_variables = true
    generate_output = 'vonmises_stress'
    material_output_order = SECOND
  []
[]
```

If you load the result into paraview you will notice that for higher order
monomials paraview automatically averages the results at the nodes. Nodal patch
recovery is available to perform such a projection more acurately inside MOOSE.
