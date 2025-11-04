# ParsedGenerateNodeset

!syntax description /Mesh/ParsedGenerateNodeset

Optionally, additional constraints can be imposed when examining a node based on :

- the subdomain of the element owning the node considered for the nodeset
- whether the node is already part of (or not part of) an existing boundary
- whether the node is 'external', e.g. it lies on the mesh exterior boundary

!alert note
A subdomain must be mutually exclusive in the two parameters: [!param](/Mesh/ParsedGenerateNodeset/included_subdomains) and [!param](/Mesh/ParsedGenerateNodeset/excluded_subdomains).
However, when both parameters are specified and the two parts of subdomains share an interface, nodes on the interface will be excluded in the newly generated node set.

!syntax parameters /Mesh/ParsedGenerateNodeset

!syntax inputs /Mesh/ParsedGenerateNodeset

!syntax children /Mesh/ParsedGenerateNodeset

