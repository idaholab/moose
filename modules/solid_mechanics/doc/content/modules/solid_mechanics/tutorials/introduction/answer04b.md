> Let's see if we can extract the x deflection of the left cantilever. It should
> be a positive value and the scalar we can extract is the maximum `disp_x`
> value in the simulation cell (the left cantilever has positive displacements
> as it bends right in the positive x direction, while the right cantilever has
> negative displacements as it bends left in the negative x direction). So we
> need a postprocessor object that gives us an extreme value of a given
> variable. Take a look at [NodalExtremeValue](NodalExtremeValue.md) and try to
> set it up to output the maximum positive x deflection.

Add the following top level block to the input

```
[Postprocessors]
  [x_deflection]
    type = NodalExtremeValue
    value_type = max
    variable = disp_x
  []
[]
```

Then update your `[Outputs]` block to look like this.

```
[Outputs]
  exodus = true
  csv = true
[]
```

When you run the simulation you should see output like this

```
Postprocessor Values:
+----------------+----------------+
| time           | x_deflection   |
+----------------+----------------+
|   0.000000e+00 |   0.000000e+00 |
|   5.000000e-01 |   2.371558e-02 |
|   1.000000e+00 |   4.744654e-02 |
|   1.500000e+00 |   7.119190e-02 |
|   2.000000e+00 |   9.495067e-02 |
|   2.500000e+00 |   1.187218e-01 |
|   3.000000e+00 |   1.425044e-01 |
|   3.500000e+00 |   1.662974e-01 |
|   4.000000e+00 |   1.900998e-01 |
|   4.500000e+00 |   2.139106e-01 |
|   5.000000e+00 |   2.377289e-01 |
+----------------+----------------+
```

We can plot this data and obtain a curve that shows the x deflection as a
function of time (and in our simulation time is proportional to the applied
pressure).

!plot scatter data=[{'x': [0.0, 0.5, 1.0, 1.5, 2.0, 2.5, 3.0, 3.5, 4.0, 4.5, 5.0],
                     'y':  [0.0, 0.023715578545891, 0.047446543911184, 0.071191904882184, 0.094950668270887, 0.11872183902512, 0.14250442033962, 0.16629741376752, 0.19009981923785, 0.21391063550499, 0.23772885980544],
                     'name':'v\=0.3'}]
              layout={'xaxis':{'title':'Time'},
                      'yaxis':{'title':'Maximum x displacement'},
                      'title':'Cantilever deflection'}
