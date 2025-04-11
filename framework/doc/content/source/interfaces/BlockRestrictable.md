# BlockRestrictable Interface

The BlockRestrictable interface is inherited by every object in MOOSE that supports running on a
subset of the mesh domain corresponding to a subdomain. This interface provides a validParameters
function that also supports a uniform way of handing reading in subdomain restrictions. Specifically,
every object may be restricted to one or more subdomains simultaneously. Subdomains may be specified
either as numeric IDs or strings (when supported by the Mesh format or named entities are used).
When the user sets `default_block` in the `[Problem]` block, the specified block restriction will be
automatically applied to all objects inheriting the BlockRestrictable interface. Users can still override
this behavior by explicitly specifying the `block` parameter within each object. If an explicitly
specified `block` is not a subset of the `default_block`, MOOSE will throw a warning
to notify the user about the inconsistency.
Finally, this interface handles queries about the set of subdomains that an object is restricted
to when no restrictions are supplied.
