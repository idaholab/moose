# Example 14 : Postprocessors and Code Verification

[](---)

## Complete Source Files

[ex14.i](https://github.com/idaholab/moose/blob/devel/examples/ex14_pps/ex14.i)

## Example 14 Output

!media large_media/examples/ex14-out.png
       caption=Example 14 Output
       style=width:50%;


```text
Postprocessor Values:
+----------------+----------------+----------------+
| time           | dofs           | integral       |
+----------------+----------------+----------------+
|   0.000000e+00 |   1.210000e+02 |   7.071068e-01 |
|   1.000000e+00 |   1.210000e+02 |   2.359249e+00 |
|   2.000000e+00 |   4.410000e+02 |   3.093980e-01 |
|   3.000000e+00 |   1.681000e+03 |   8.861951e-02 |
|   4.000000e+00 |   6.561000e+03 |   2.297902e-02 |
|   5.000000e+00 |   2.592100e+04 |   5.797875e-03 |
|   6.000000e+00 |   1.030410e+05 |   1.452813e-03 |
+----------------+----------------+----------------+
```

!media large_media/examples/ex14-conv-rate.png
       style=width:50%;

[](---)

## Comparison to a Fine Grid Solution

Also present in Example 14 are two input files (`ex14_solution_comparison_1.i` and `ex14_solution_comparison_1.i`) that demonstrate how to use a [SolutionUserObject](source/userobjects/SolutionUserObject.md) to read in a fine grid solution and then compare a coarse grid solution to that using `ElementL2Error`.

- The first input file computes the fine grid solution and outputs an XDA file.
- An XDA file contains the full set of sata necessary to perfectly read in a previous solution...even on adapted meshes.
- The second inout file uses `SolutionUserObject` to read in the fine-grid solution.
- Next, a `SolutionFunction` utilizes the `SolutionUserObject` to present the solution field as a MOOSE `Function`.
- Finally, an `ElementL2Error``Postprocessor` is utilized to compute the difference between the solutions.

[ex14_compare_solutions_1.i](https://github.com/idaholab/moose/blob/devel/examples/ex14_pps/ex14_compare_solutions_1.i)

[ex14_compare_solutions_2.i](https://github.com/idaholab/moose/blob/devel/examples/ex14_pps/ex14_compare_solutions_2.i)

!content pagination use_title=True
                    previous=examples/ex13_functions.md
                    next=examples/ex15_actions.md
