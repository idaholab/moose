# SubdomainsGeneratorBase

This is a base class that can be used for mesh generators that use a flood algorithm
on elements.
The flood elements recursively progresses from an element to its neighbors and performs
an operation.

!alert note
It is currently only implemented to handle surface elements.

!alert note
The only operation currently implemented are changing the subdomain ID of the element,
and flipping its orientation if the surface element normal is opposite the painting/flooding normal.

Several techniques and restrictions are implemented for this flooding algorithm:

- propagating only to surface elements within a list of included subdomains
- propagating only to surface elements of a fixed normal
- propagating from surface element to neighbors with a change in the normal below the tolerance
- propagating from surface element to neighbors with a change in the normal below a specified tolerance and with a sign flip in the normal
- propagating from surface element to neighbors within a certain distance to the starting element
