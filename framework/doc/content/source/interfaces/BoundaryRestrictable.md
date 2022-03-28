# BoundaryRestrictable Interface

The BoundaryRestrictable interface is inherited by every object in MOOSE that supports running on a subset
of the mesh domain corresponding to a boundary. This interface provides a validParameters function that also
supports a uniform way of handing reading in boundary restrictions. Specifically, every object may be
restricted to one or more boundaries simultaneously. Boundaries may be specified either as numeric IDs
or strings (when supported by the Mesh format or named entities are used). Finally, this interface handles
queries about the set of boundaries that an object is restricted to when no restrictions are supplied.
