# ElementExtremeMaterialProperty

This postprocessor computes the minimum or maximum of a material property from
all quadrature points in a domain.

## Example input syntax

In this example, the minimum and maximum of the material property `mat_prop` are being sampled
by two `ElementExtremeMaterialProperty` postprocessors.

!listing test/tests/postprocessors/element_extreme_material_property/element_extreme_material_property.i block=Materials Postprocessors

!syntax parameters /Postprocessors/ElementExtremeMaterialProperty

!syntax inputs /Postprocessors/ElementExtremeMaterialProperty

!syntax children /Postprocessors/ElementExtremeMaterialProperty
