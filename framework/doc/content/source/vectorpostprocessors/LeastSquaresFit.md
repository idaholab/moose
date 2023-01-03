# LeastSquaresFit

!syntax description /VectorPostprocessors/LeastSquaresFit

## Description

`LeastSquaresFit` is used perform a polynomial least squares fit of data provided through another VectorPostprocessor. It computes the coefficients for a polynomial of arbitrary, user-specified order that minimize the error using a standard least-squares procedure.  This object provides the option of either outputting the polynomial coefficients of the least squares fit as a single vector, or outputting a user-defined number of samples from the polynomial using the fitted coefficients.

This VectorPostprocessor is closely related to the [LeastSquaresFitHistory](/LeastSquaresFitHistory.md) VectorPostprocessor, which performs the same type of least squares fit, but stores the results in a set of vectors that store the full history of the individual coefficients over a transient analysis.

The vectors of values of the independent ($x$) and dependent ($y$) variables on which the least squares fit is performed are provided through another VectorPostprocessor, which must provide two equally-sized vectors of data upon which to operate.  The name of this VectorPostprocessor is provided using the `vectorpostprocessor` parameter, and the names of the data vectors are provided with the `x_name` and `y_name` parameters. The vectors of data can be shifted and/or scaled through the use of optional parameters.

By default, if an insufficient number of points is provided in these data vectors, the order of the polynomial will be truncated to one less than the number of points. If the `truncate_order   parameter is set to `false`, an error will be generated in this case.

The user must define whether the output should be in the form of polynomial coefficients or samples using the `output` parameter. If the option to output polynomial coefficients is used, they are stored in a vector named `coefficients`. If samples are requested, the names of the sample vectors are the same as those of the data specified by `x_name` and `y_name`.

!syntax parameters /VectorPostprocessors/LeastSquaresFit

!syntax inputs /VectorPostprocessors/LeastSquaresFit

!syntax children /VectorPostprocessors/LeastSquaresFit
