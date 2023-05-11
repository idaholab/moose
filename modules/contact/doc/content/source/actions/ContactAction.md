# Contact Action

!syntax description /Contact/ContactAction

## Description

ContactAction is a MOOSE action that constructs objects needed for mechanical contact enforcement. This
is invoked by including the [Contact](syntax/Contact/index.md) block at the top level in a MOOSE input file.
See the page documenting the syntax for that block for a description, example usage, and parameters.

For node-to-segment mechanical contact, the action offers the possibility to automatically set up
mechanical contact pairs given a maximum distance between contacting boundary centroids.
The user can leverage this capability by providing `automatic_pairing_distance` and
`automatic_pairing_boundaries`. This is particularly useful when many feasible contact
interactions can take place in a periodically repeating pattern.
