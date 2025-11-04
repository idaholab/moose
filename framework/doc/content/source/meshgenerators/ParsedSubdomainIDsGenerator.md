# ParsedSubdomainIDsGenerator

!syntax description /Mesh/ParsedSubdomainIDsGenerator

## Overview

The `ParsedSubdomainIDsGenerator` generator allows the user to specify parsed expressions to compute the subdomain IDs of elements in the mesh.

The parsed expression is provided by [!param](/Mesh/ParsedSubdomainIDsGenerator/expression), which can a function of the element vertex average coordinates `x`, `y`, and `z`. More importantly, users can use the extra element integer values of each element as the function variables. The names of these extra element integers can be provide through [!param](/Mesh/ParsedSubdomainIDsGenerator/extra_element_id_names). This feature allows users to assign subdomain IDs based on the values of these extra element integers.

When the subdomain IDs are reassigned by this generator, [!param](/Mesh/ParsedSubdomainIDsGenerator/excluded_subdomains) can be used to specify a list of subdomains that need to be retained.

Note that due the flexibility provided by the parsed expression, the function value could be a non-integer value or negative. If a negative value is evaluated for an involved element in the mesh, the mesh generator will throw an error. On the other hand, if a non-integer value is evaluated, the rounded function value is used as the subdomain ID.

!syntax parameters /Mesh/ParsedSubdomainIDsGenerator

!syntax inputs /Mesh/ParsedSubdomainIDsGenerator

!syntax children /Mesh/ParsedSubdomainIDsGenerator
