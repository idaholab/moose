# Frictionless contact algorithm comparison

| Lagrange multiplier | Displacement | NCP function | Time (arbitrary units) | Time steps | Nonlinear iterations |
| ------------------- | ------------ | ------------ | ---------------------- | ---------- | -------------------- |
| Nodal | Mortar | Min | 14.401 | 40 | 104 |
| Nodal | Mortar | FB | 17.752 | 40 | 135 |
| Nodal | Nodal | Min | 5.438 | 41 | 104 |
| Nodal | Nodal | FB | 6.770 | 41 | 149 |
| Mortar | Mortar | Min | 14.454 | 40 | 106 |
| Mortar | Mortar | FB | 19.027 | 40 | 136 |

## Notes

- Clearly having mortar mesh generation slows the simulation down, which is not surprising
- The min NCP function is undeniably better (at least here in the frictionless case)
- For the pure nodal algorithms, the time step that did not converge featured classic ping-ponging behavior:

```
 5 Nonlinear |R| = 4.007951e-04
    |residual|_2 of individual variables:
                  disp_x:    0.000399808
                  disp_y:    2.75599e-05
                  normal_lm: 5.52166e-06


The number of nodes in contact is 11

      0 Linear |R| = 4.007951e-04
      1 Linear |R| = 1.287307e-04
      2 Linear |R| = 8.423398e-06
      3 Linear |R| = 1.046825e-07
      4 Linear |R| = 8.017310e-09
      5 Linear |R| = 3.053040e-10
  Linear solve converged due to CONVERGED_RTOL iterations 5
 6 Nonlinear |R| = 4.432193e-04
    |residual|_2 of individual variables:
                  disp_x:    0.000396694
                  disp_y:    0.00019545
                  normal_lm: 2.96013e-05


The number of nodes in contact is 11

      0 Linear |R| = 4.432193e-04
      1 Linear |R| = 1.355935e-04
      2 Linear |R| = 1.216010e-05
      3 Linear |R| = 6.386952e-07
      4 Linear |R| = 2.235594e-08
      5 Linear |R| = 2.884193e-10
  Linear solve converged due to CONVERGED_RTOL iterations 5
 7 Nonlinear |R| = 4.008045e-04
    |residual|_2 of individual variables:
                  disp_x:    0.000399816
                  disp_y:    2.76329e-05
                  normal_lm: 5.29313e-06


The number of nodes in contact is 11

      0 Linear |R| = 4.008045e-04
      1 Linear |R| = 1.287272e-04
      2 Linear |R| = 8.423081e-06
      3 Linear |R| = 1.047782e-07
      4 Linear |R| = 8.054781e-09
      5 Linear |R| = 3.046073e-10
  Linear solve converged due to CONVERGED_RTOL iterations 5
 8 Nonlinear |R| = 4.432194e-04
```

# Preliminary frictional contact algorithm comparison

| LM normal | LM tangential | Displacement | NCP function normal | NCP function tangential | Time (arbitrary units) | Time steps | Nonlinear iterations |
| --------- | ------------  | ------------ | ------------------- | ----------------------- | ---------------------- | ---------- | -------------------- |
| Mortar | Mortar | Mortar | FB | FB | 18.771 | 43 | 190 |
| Mortar | Mortar | Mortar | min | FB | 18.281 | 41 | 194 |
| Nodal | Mortar | Mortar | min | FB | 15.544 | 41 | 165 |
| Mortar | Mortar | Mortar | Min | Min | 74.801 | 58 | 320 |
| Nodal | Nodal | Mortar | Min | Min | 36.923 | 52 | 230 |
| Nodal | Nodal | Mortar | FB | FB | NA | NA | NA |
