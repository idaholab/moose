# Eigenvalues

!syntax description /VectorPostprocessors/Eigenvalues

This vector postprocessor returns the real and imaginary part of each eigenvalue that was computed by `Eigen`. These form the two columns when output to CSV, and the eigenvalues are sorted by rows.

!alert note title=Vector names / CSV output column names
The vectors declared by the `Eigenvalues` vector postprocessor object are named `eigen_values_real` and `eigen_values_imag` for, respectively, the real and imaginary parts of the eigenvalues.

## Natural frequencies

Setting [!param](/VectorPostprocessors/Eigenvalues/natural_frequency) to `true` declares three more vectors, each computed from the real part $\lambda_r$ of the corresponding eigenvalue:

\begin{equation}
\omega = \sqrt{\lambda_r}, \qquad f = \frac{\omega}{2 \pi}, \qquad T = \frac{1}{f}
\end{equation}

| Vector | Symbol | Unit |
| :- | :- | :- |
| `angular_frequency` | $\omega$ | rad/s |
| `frequency` | $f$ | Hz |
| `period` | $T$ | s |

The relations above carry no conversion factor, so the units listed hold when the stiffness and mass terms of the eigenvalue problem are expressed in SI units. These columns are meaningful for a modal analysis, in which the eigenvalue the [Eigenvalue.md] executioner solves for is the square of the angular frequency; see [1D elastic waves](modules/solid_mechanics/1d_elastic_waves.md) for such a setup. The imaginary part of the eigenvalue takes no part in these three columns and stays in `eigen_values_imag`.

The three vectors are declared only when [!param](/VectorPostprocessors/Eigenvalues/natural_frequency) is `true`. An input that leaves it at its default therefore writes exactly the two eigenvalue columns to CSV.

An eigenvalue with a negative real part has no real frequency. That row is filled with `NaN` in all three columns, and one warning per evaluation reports how many eigenvalues were negative. A negative eigenvalue usually means the mass term was assembled with the wrong sign, so treat the warning as a setup error rather than a result. A zero real part, which a rigid body mode produces, gives $\omega = f = 0$ and an infinite period without a warning.

[!param](/VectorPostprocessors/Eigenvalues/natural_frequency) cannot be combined with [!param](/VectorPostprocessors/Eigenvalues/inverse_eigenvalue), because the inverse of an eigenvalue has no frequency meaning. Requesting both is an error.

## Example input syntax

In this input file, the variable `u` is the solution of an eigenvalue diffusion-reaction problem. An `Eigenvalue` executioner is used to compute the eigenvalue of the system, which is retrieved by the `Eigenvalues` vector postprocessor.

!listing test/tests/problems/eigen_problem/eigensolvers/ne.i block=VectorPostprocessors

The next input file solves for the three lowest longitudinal modes of an elastic bar and asks for the frequency columns as well.

!listing modules/solid_mechanics/test/tests/modal_analysis/fixed_fixed_bar.i block=VectorPostprocessors

!syntax parameters /VectorPostprocessors/Eigenvalues

!syntax inputs /VectorPostprocessors/Eigenvalues

!syntax children /VectorPostprocessors/Eigenvalues
