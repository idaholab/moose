# ElementIDInterface

Extra integer IDs for all the elements of a mesh can be useful for handling complicated material assignment, performing specific calculations on groups of elements, etc.
Often times, we do not want to use subdomain IDs for these tasks because subdomains are used by MOOSE for block-restricted operations. Many subdomains could introduce large penalty on run-time performance.
Extra element IDs are identified with extra element ID names.
Extra element IDs are part of the mesh and can be recovered during the mesh recovery.
Please refer to [MeshGenerator](MeshGenerator.md) for how the extra element integer IDs are generated or imported.

The ElementIDInterface is designed for using extra element IDs in the mesh by MOOSE objects such as materials, user objects, aux kernels, DG kernels, interface kernels, IC (initial conditions), kernels, etc.
The following table summarizes the interface functions provided by this interface:

| Method | Description |
| - | - |
getElementIDIndex | Gets index of an element integer with an input parameter of the integer names
getElementIDIndexByName | Gets index of an element integer with its name
getElementID | Gets the constant reference of an element integer of the current element with an input parameter
getElementIDNeighbor | Gets the constant reference of an element integer of the neighbor element with an input parameter
getElementIDByName | Gets the constant reference of an element integer of the current element with the integer name
getElementIDNeighborByName | Gets the constant reference of an element integer of the neighbor element with the integer name
hasElementID | Whether mesh has an element integer with a given name
maxElementID | Gets the maximum element ID for an element integer with its index
minElementID | Gets the minimum element ID for an element integer with its index
areElemIDsIdentical | Whether two element integers are identical for all elements
getAllElemIDs | Gets all the unique element IDs for an element integer with its index on the entire domain
getElemIDsOnBlocks | Gets all the unique element IDs for an extra element integer with its index on a set of subdomains
getElementID | Gets an element integer for an element

Because MOOSE creates three copies of materials for the current element, element face and neighboring element face ([Material](syntax/Materials/index.md)), `getElementID` and `getElementIDByName` return the reference to either the current element ID or the neighbor element ID based on whether the current copy of material is for neighboring element or not. Correspondingly, directly calling `getElementIDNeighbor` and `getElementIDNeighborByName` is not allowed in `Material`.


It is noted that the element integer name *subdomain_id* is reserved by MOOSE for accessing subdomain IDs with this interface.
