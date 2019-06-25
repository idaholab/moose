# SystemBase

The SystemBase is an abstract class that represents a system of equations. It is the
basis for the [NonlinearSystem.md], [AuxiliarySystem.md], and DisplacedSystem objects.
Each System contains the necessary objects for holding a mesh, one or more field
variables (discretized on that mesh) and one or more scalar variables (DOFs not directly
associated with the mesh). Access to raw numeric vectors, which contain solution
information all go through this class and its derived objects.