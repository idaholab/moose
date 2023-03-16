# BatchMaterial

`BatchMaterial` implements a generic base class for a userobject that can gather MaterialProperties and Variables from all QPs in the local domain to perform a computation in a single batch (offloaded to a GPU for example).

The number and types of properties and variables is flexible and will be resolved at compile time through templating.

The resulting userobject will generate an "array of structs" for optimal cache locality in the batch computation. The input data "struct" is a tuple. The base class is templated on a tuple wrapper class. This allows use of either `std::tuple` or `cuda::std::tuple` (which has corresponding implementations on the host and the device with identical memory layouts!).

The output data type is a user specified template parameter (it could be a struct or a simple number, etc.). A map is maintained by the user object to enable easy lookup of element/qp data in the output array.

The user must write a simple glue material class that uses the `getOutputData()`/`getIndex(elem_id)` interface of the derived batch material user object.

