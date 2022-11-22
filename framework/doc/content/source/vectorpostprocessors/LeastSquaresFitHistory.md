# LeastSquaresFitHistory

!syntax description /VectorPostprocessors/LeastSquaresFitHistory

## Description

`LeastSquaresFitHistory` is used perform a polynomial least squares fit of data provided through another VectorPostprocessor. It computes the coefficients for a polynomial of arbitrary, user-specified order that minimize the error using a standard least-squares procedure.This object stores the polynomial coefficients in a set of vectors that contain the full history of those values for a transient analysis.

This VectorPostprocessor is closely related to the [LeastSquaresFit](/LeastSquaresFit.md) VectorPostprocessor, which performs the same type of least squares fit, but stores the results in a single vector, the history of which is not stored.

The polynomial coefficients are stored in a set of vectors named `coef_0` through `coef_n`, where $n$ is the specified order of the polynomial. Each of these vectors stores the full history of its coefficient in time, so that the combination of component $i$ for each of these vectors defines the polynomial fit for time step $i$. In addition, a vector named `time` stores the values of the solution time for each step of a transient analysis.

The vectors of values of the independent ($x$) and dependent ($y$) variables on which the least squares fit is performed are provided through another VectorPostprocessor, which must provide two equally-sized vectors of data upon which to operate.  The name of this VectorPostprocessor is provided using the `vectorpostprocessor` parameter, and the names of the data vectors are provided with the `x_name` and `y_name` parameters. The vectors of data can be shifted and/or scaled through the use of optional parameters.

By default, if an insufficient number of points is provided in these data vectors, the order of the polynomial will be truncated to one less than the number of points. If the `truncate_order   parameter is set to `false`, an error will be generated in this case.

!syntax parameters /VectorPostprocessors/LeastSquaresFitHistory

!syntax inputs /VectorPostprocessors/LeastSquaresFitHistory

!syntax children /VectorPostprocessors/LeastSquaresFitHistory
