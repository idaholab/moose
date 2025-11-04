# ParsedExtraElementIDGenerator

!syntax description /Mesh/ParsedExtraElementIDGenerator

This mesh generator adds an extra element integer in an input mesh based on an parsed expression evaluated at the ceontroid of every element.
The extra element integer can be added for elements only within certain mesh subdomains using the [!param](/Mesh/ParsedExtraElementIDGenerator/restricted_subdomains) parameter.
Other extra element integers, already existing in the mesh, can also be used in the expression.
"x", "y", "z" as coordinates can be used in the expression.
"pi" for the ratio of a circle's circumference to its diameter, "e" for the exponential constant can also be used in the expression.
"invalid_elem_id" can be used in the expression for an invalid value of extra element integers.

!alert note title=Invalid ids
An expression of this mesh generator may generate "invalid_elem_id" (equivalent to the extra element ID not being set), if and only if the expression giving real numbers rounds to the invalid id (either a 4-byte or a 8-byte integer set at the compile time).

!syntax parameters /Mesh/ParsedExtraElementIDGenerator

!syntax inputs /Mesh/ParsedExtraElementIDGenerator

!syntax children /Mesh/ParsedExtraElementIDGenerator
