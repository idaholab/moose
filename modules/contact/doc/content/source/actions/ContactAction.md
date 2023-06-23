# Contact Action

!syntax description /Contact/ContactAction

## Description

ContactAction is a MOOSE action that constructs objects needed for mechanical contact enforcement. This
is invoked by including the [Contact](syntax/Contact/index.md) block at the top level in a MOOSE input file.
See the page documenting the syntax for that block for a description, example usage, and parameters.

For node-to-segment mechanical contact, the action offers the possibility to automatically set up
mechanical contact pairs given a maximum distance between contacting boundary centroids.
To use that option, the user must set `automatic_pairing_method = CENTROID`.
The user can leverage this capability by providing `automatic_pairing_distance` and
`automatic_pairing_boundaries`. This is particularly useful when many feasible contact
interactions can take place in a periodically repeating pattern.

Alternatively, also for
node-to-segment, the user can choose to select a computation of proximity based on nodal
locations. In essence, for all boundaries provided by the user in `automatic_pairing_boundaries`,
the action will search for all nodes whose distance is less than `automatic_pairing_distance`.
If so, each nodal pair distance from different boundaries less than the `automatic_pairing_distance`
distance will create a contact pair. Repeated contact pairs are automatically eliminated. In order to
activate this feature, in addition to `automatic_pairing_boundaries` and `automatic_pairing_distance`, the
user needs to set the input parameter `automatic_pairing_method = NODE`.
