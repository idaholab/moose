# MultiAppGeneralFieldUserObjectTransfer

!syntax description /Transfers/MultiAppGeneralFieldUserObjectTransfer

## Description

`MultiAppGeneralFieldUserObjectTransfer` transfers information from a `UserObject` in an application to a variable in another.
This transfer is a [General Field](MultiAppGeneralFieldTransfer.md) version of the [MultiAppUserObjectTransfer.md]. As such it
supports numerous additional features (various spatial restrictions, higher order variables, etc), listed in the base class documentation.

If the target domain is not fully enclosed inside the source app meshes, the user objects will still be queried at each target point,
and, only if the values is valid, will be used. This is the case both for interpolating between source domains and extrapolating.

!alert note
The user object must implement the `spatialValue` routine, which is used to obtain the UO value at candidate origin points in
the source application.

!syntax parameters /Transfers/MultiAppGeneralFieldUserObjectTransfer

!syntax inputs /Transfers/MultiAppGeneralFieldUserObjectTransfer

!syntax children /Transfers/MultiAppGeneralFieldUserObjectTransfer
