# MFEMTopology

!if! function=hasCapability('mfem')

## Overview

`MFEMTopology` is responsible for providing an interface to methods that return topological
information about an `mfem::Mesh` object, such as mappings between topologically equivalent vertices
in a mesh constrained by discrete symmetries that can be used in enforcing periodic boundary
conditions.

!if-end!

!else
!include mfem/mfem_warning.md
