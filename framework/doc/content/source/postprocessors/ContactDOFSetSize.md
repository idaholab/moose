# ContactDOFSetSize

The `ContactDOFSetSize` class outputs the number of degrees of freedom greater than
a certain tolerance (specified through the `tolerance` parameter; the default
value is 1e-6). The usual application of this, as indicated by the class name,
is to indicate how many nodes (if using first order Lagrange shape functions for
the contact pressure lagrange multiplier) or element faces (if using constant
monomials) are in contact. The `subdomain` parameter should be the name or id
representing the lower dimensional block that the Lagrange multiplier variable
lives on.

# Description and Syntax

!syntax description /Postprocessors/ContactDOFSetSize

!syntax parameters /Postprocessors/ContactDOFSetSize

!syntax inputs /Postprocessors/ContactDOFSetSize

!syntax children /Postprocessors/ContactDOFSetSize

!bibtex bibliography
