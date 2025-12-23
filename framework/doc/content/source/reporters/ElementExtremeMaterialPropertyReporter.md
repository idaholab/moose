# ElementExtremeMaterialPropertyReporter

This reporter computes the minimum or maximum value of a specified material property from
all volumetric quadrature points. It reports that extreme value, as well as the coordinates
of the point with the extreme value. In addition, it can optionally report the values of
other material properties at that point.

## Example input syntax

In this example, the minimum and maximum of the material property `mat_prop` are being sampled
by two `ElementExtremeMaterialPropertyReporter` objects. The values of other material properties
are also reported at the locations of those extreme values.

!listing test/tests/reporters/element_extreme_material_property/element_extreme_material_property.i block=Materials Reporters

!syntax parameters /Reporters/ElementExtremeMaterialPropertyReporter

!syntax inputs /Reporters/ElementExtremeMaterialPropertyReporter

!syntax children /Reporters/ElementExtremeMaterialPropertyReporter
