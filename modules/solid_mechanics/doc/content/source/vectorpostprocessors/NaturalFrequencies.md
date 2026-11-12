# NaturalFrequencies

!syntax description /VectorPostprocessors/NaturalFrequencies

## Description

`NaturalFrequencies` reports the eigenvalues computed by the [Eigenvalue.md] executioner, as the
[Eigenvalues.md] vector postprocessor does, and converts each one into the angular frequency,
frequency, and period of its mode. In a modal analysis the eigenvalue problem is
$\mathbf{K} \mathbf{u} = \omega^2 \mathbf{M} \mathbf{u}$, with stiffness $\mathbf{K}$, mass
$\mathbf{M}$, and mode shape $\mathbf{u}$, so each eigenvalue is $\omega^2$. From the real part
$\lambda_r$ of each eigenvalue,

\begin{equation}
\omega = \sqrt{\lambda_r}, \qquad f = \frac{\omega}{2 \pi}, \qquad T = \frac{1}{f}
\end{equation}

The object always writes five vectors, one row per eigenvalue:

| Vector | Symbol | Unit |
| :- | :- | :- |
| `eigen_values_real` | $\lambda_r$ | rad$^2$/s$^2$ |
| `eigen_values_imag` | $\lambda_i$ | rad$^2$/s$^2$ |
| `angular_frequency` | $\omega$ | rad/s |
| `frequency` | $f$ | Hz |
| `period` | $T$ | s |

The relations carry no conversion factor. The units listed hold when the stiffness and mass terms
use a consistent unit system whose time unit is the second.

The base parameter `inverse_eigenvalue` is not available, because the inverse of an eigenvalue has
no frequency meaning.

## Invalid and zero eigenvalues

An eigenvalue with a negative real part or a nonzero imaginary part has no real frequency of this
form. Its row is `NaN` in the `angular_frequency`, `frequency`, and `period` vectors, and one
warning per evaluation reports how many eigenvalues were affected. A negative eigenvalue usually
means the mass term has the wrong sign, so treat the warning as a setup error.

An eigensolver returns the zero eigenvalue of a rigid body mode as a small number of either sign.
A real part whose magnitude is not above
[!param](/VectorPostprocessors/NaturalFrequencies/zero_eigenvalue_tolerance) times the largest
real-part magnitude among the converged eigenvalues is treated as zero. That mode gives
$\omega = f = 0$ and an infinite period, without a warning. The scale is the largest converged
eigenvalue, so a solve that converges only near-zero eigenvalues treats none of them as zero.

## Example Input File Syntax

This input solves for the three lowest longitudinal modes of an elastic bar fixed at both ends.

!listing modules/solid_mechanics/test/tests/modal_analysis/fixed_fixed_bar.i block=VectorPostprocessors

!syntax parameters /VectorPostprocessors/NaturalFrequencies

!syntax inputs /VectorPostprocessors/NaturalFrequencies

!syntax children /VectorPostprocessors/NaturalFrequencies
