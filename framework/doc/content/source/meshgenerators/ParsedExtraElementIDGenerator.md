# ParsedExtraElementIDGenerator

!syntax description /Mesh/ParsedExtraElementIDGenerator

This mesh generator adds an extra element integer in an input mesh based on an parsed expression evaluated at the ceontroid of every element.
The extra element integer can be added for elements only within certain mesh subdomains using the [!param](/Mesh/ParsedExtraElementIDGenerator/restricted_subdomains) parameter.
Other extra element integers, already existing in the mesh, can also be used in the expression.

!syntax parameters /Mesh/ParsedExtraElementIDGenerator

!syntax inputs /Mesh/ParsedExtraElementIDGenerator

!syntax children /Mesh/ParsedExtraElementIDGenerator
