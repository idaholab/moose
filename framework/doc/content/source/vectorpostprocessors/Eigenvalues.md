# Eigenvalues

!syntax description /VectorPostprocessors/Eigenvalues

This vector postprocessor returns the real and imaginary part of each eigenvalue that was computed by `Eigen`. These form the two columns when output to CSV, and the eigenvalues are sorted by rows.

!alert note title=Vector names / CSV output column names
The vectors declared by the `Eigenvalues` vector postprocessor object are named `eigen_values_real` and `eigen_values_imag` for, respectively, the real and imaginary parts of the eigenvalues.

## Example input syntax

In this input file, the variable `u` is the solution of an eigenvalue diffusion-reaction problem. An `Eigenvalue` executioner is used to compute the eigenvalue of the system, which is retrieved by the `Eigenvalues` vector postprocessor.

!listing test/tests/problems/eigen_problem/eigensolvers/ne.i block=VectorPostprocessors

!syntax parameters /VectorPostprocessors/Eigenvalues

!syntax inputs /VectorPostprocessors/Eigenvalues

!syntax children /VectorPostprocessors/Eigenvalues
