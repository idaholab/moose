# NearestNodeLocator

- NearestNodeLocator provides the nearest node on a "Master" boundary for each node on a "Slave" boundary (and the other way around).
- The distance between the two nodes is also provided.
- It works by generating a "Neighborhood" of nodes on the Master side that are close to the Slave node.
- The size of the Neighborhood can be controlled in the input file by setting the `patch_size` parameter in the `Mesh` section.

!media media/geomsearch/nearest_node_diagram.jpg

- To use a NearestNodeLocator
    - `#include "NearestNodeLocator.h"`
    - call `getNearestNodeLocator(master_id, secondary_id)` to create the object.
- The functions `distance()` and `nearestNode()` both take a node ID and return either the distance to the nearest node or a `Node` pointer for the nearest node respectively.
