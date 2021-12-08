# VerifyNodalUniqueID

!syntax description /UserObjects/VerifyNodalUniqueID

This object is used for debugging mesh issues.

!alert note
For distributed mesh, this will perform an `MPI_AllGather` operation, sending all ids to all processes, which may require a lot of memory on all processes.

!syntax parameters /UserObjects/VerifyNodalUniqueID

!syntax inputs /UserObjects/VerifyNodalUniqueID

!syntax children /UserObjects/VerifyNodalUniqueID
