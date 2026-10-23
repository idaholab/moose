# LibmeshMFEMMesh

!if! function=hasCapability('mfem')

This is a subclass of `mfem::Mesh` and is used for constructing an
MFEM-based mesh from information describing a `libMesh`-based
mesh. Ideally all of this would be done through passing arguments to
the constructor `mfem::Mesh` and calling its public
methods. Unfortunately, not all necessary functionality is currently
accessible through the public API. As such it is necessary to write a
subclass that can call protected methods and modify protected members.

For more details on the overall process of creating an `mfem::Mesh`
from a `MooseMesh` object, see the [documentation on
`buildMFEMMesh`](source/mfem/utils/BuildMFEMMesh.md).

!if-end!

!else
!include mfem/mfem_warning.md
