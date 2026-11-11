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

The relations above carry no conversion factor, so the units listed hold when the stiffness and mass terms of the eigenvalue problem are expressed in SI units. These columns are meaningful for a modal analysis, in which the eigenvalue the [Eigenvalue.md] executioner solves for is the square of the angular frequency; the solid mechanics module's frequency domain analysis page shows such a setup.

An input that does not set [!param](/VectorPostprocessors/Eigenvalues/natural_frequency) writes only the two eigenvalue columns to CSV.

An eigenvalue with a negative real part or a nonzero imaginary part has no real frequency of this form. That row is filled with `NaN` in all three columns, and one warning per evaluation reports how many eigenvalues were affected. A negative eigenvalue usually means the mass term was assembled with the wrong sign, so treat the warning as a setup error rather than a result.

An eigensolver returns the zero eigenvalue of a rigid body mode as a small number of either sign. An eigenvalue is therefore treated as zero when the magnitude of its real part is not above [!param](/VectorPostprocessors/Eigenvalues/zero_eigenvalue_tolerance) times the largest real-part magnitude among the converged eigenvalues. Such a mode gives $\omega = f = 0$ and an infinite period without a warning. The scale is the largest converged eigenvalue, so a solve that converges only near-zero eigenvalues treats none of them as zero.

[!param](/VectorPostprocessors/Eigenvalues/natural_frequency) cannot be combined with [!param](/VectorPostprocessors/Eigenvalues/inverse_eigenvalue), because the inverse of an eigenvalue has no frequency meaning. Requesting both is an error.

## Example input syntax

In this input file, the variable `u` is the solution of an eigenvalue diffusion-reaction problem. An `Eigenvalue` executioner is used to compute the eigenvalue of the system, which is retrieved by the `Eigenvalues` vector postprocessor.

!listing test/tests/problems/eigen_problem/eigensolvers/ne.i block=VectorPostprocessors

The next input file solves for the three lowest longitudinal modes of an elastic bar and asks for the frequency columns as well.

!listing modules/solid_mechanics/test/tests/modal_analysis/fixed_fixed_bar.i block=VectorPostprocessors

!syntax parameters /VectorPostprocessors/Eigenvalues

!syntax inputs /VectorPostprocessors/Eigenvalues

!syntax children /VectorPostprocessors/Eigenvalues
