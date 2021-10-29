> Look at the
> [`NodalValueSampler`](NodalValueSampler.md)[vectorpostprocessor](VectorPostprocessors/index.md)
> and see if you can use its
> [!param](/VectorPostprocessors/NodalValueSampler/block) parameter to output the
> `pillars_normal_lm` variable on the `pillars_secondary_subdomain` subdomain.

Add the following block to the `step02.i` input

```
[VectorPostprocessors]
  [normal0]
    type = NodalValueSampler
    variable = pillars_normal_lm
    block = pillars_secondary_subdomain
    sort_by = id
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]
```

In the `[Outputs]` block add the `csv = true` parameter. Run the input and you
should end up with the new output file `step02_out_normal0_0000.csv` through
`step02_out_normal0_0010.csv` (one output for each time step plus one for the
initial condition).

In the CSV files you will find multiple columns. The rows are ordered according
to node ID, which should result in a continuous path along the mortar subdomain.
You can also order the rows by any coordinate component.

In the same directory as the tutorial inputs you will find a small python script
to plot the contact pressure (using matplotlib and pandas). It should serve as a
starting point for custom plot generation.

Note that since we did not explicitly request the `NodalValueSampler` to use the
displaced mesh with `use_displaced_mesh = true` the coordinates in the CSV file
refer to the undeformed positions along the contact surface.

Try increasing the [!param](/Executioner/Transient/end_time) to 10 and replot the
contact pressures with the supplied script.
