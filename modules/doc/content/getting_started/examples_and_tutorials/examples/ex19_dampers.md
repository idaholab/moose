# Example 19 : Newton Damping

[ex19.i](https://github.com/idaholab/moose/blob/devel/examples/ex19_dampers/ex19.i)

[](---)

## Example 19 Results

Damping = 1.0

```text
    Outputting Initial Condition
      True Initial Nonlinear Residual: 10.4403
      NL step  0, |residual|_2 = 1.044031e+01
      NL step  1, |residual|_2 = 6.366756e-05
      NL step  2, |residual|_2 = 3.128450e-10
    0: 3.128450e-10
```

Damping = 0.9

```text
    Outputting Initial Condition
      True Initial Nonlinear Residual: 10.4403
      NL step  0, |residual|_2 = 1.044031e+01
      NL step  1, |residual|_2 = 1.044031e+00
      NL step  2, |residual|_2 = 1.044031e-01
      NL step  3, |residual|_2 = 1.044031e-02
      NL step  4, |residual|_2 = 1.044031e-03
      NL step  5, |residual|_2 = 1.044031e-04
      NL step  6, |residual|_2 = 1.044031e-05
      NL step  7, |residual|_2 = 1.044031e-06
      NL step  8, |residual|_2 = 1.044031e-07
      NL step  9, |residual|_2 = 1.044031e-08
    0: 1.044031e-08
```

!content pagination use_title=True
                    previous=examples/ex18_scalar_kernel.md
                    next=examples/ex20_user_objects.md
