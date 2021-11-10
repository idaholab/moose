# ElementAverageValue

!syntax description /Postprocessors/ElementAverageValue

The element average of a variable is defined as the ratio of its integral over the volume of the domain:

!equation
\bar{u} = \dfrac{\int_{\Omega} u d\Omega}{\int_{\Omega} d\Omega}

This average may be taken over a restriction of blocks, but for taking an average over boundaries,
a [SideAverageValue.md] should be used instead.

## Example input syntax

In this example, we compute the average of a variable $u$ over the whole domain.

!listing test/tests/postprocessors/element_average_value/element_average_value_test.i block=Postprocessors

!syntax parameters /Postprocessors/ElementAverageValue

!syntax inputs /Postprocessors/ElementAverageValue

!syntax children /Postprocessors/ElementAverageValue
