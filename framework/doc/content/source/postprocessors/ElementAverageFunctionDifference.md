# ElementAverageFunctionDifference

!syntax description /Postprocessors/ElementAverageFunctionDifference

# Description and Syntax

The `ElementAverageFunctionDifference` is a `ElementIntegralPostprocessor` that computes the
difference between the elemental average of a variable with a function evaluated at a given point,
\begin{equation}
  u_{avg} - f(t,x,y,z).
\end{equation}

The `absolute_value` input parameter optionally returns the absolute value of the difference,
\begin{equation}
  |(|u_{avg}| - |f(t,x,y,z)|)|.
\end{equation}

!listing test/tests/postprocessors/element_average_function_difference/test.i block=Postprocessors

!syntax parameters /Postprocessors/ElementAverageFunctionDifference

!syntax inputs /Postprocessors/ElementAverageFunctionDifference

!syntax children /Postprocessors/ElementAverageFunctionDifference

!bibtex bibliography
