# InterfaceUserObject

This User Object provides the base class to make computation across an interface.
Specifically it allows to probe both material property and variables on both sides
simply. This class is very similar to `InternalSideUserobject`, except that  is
boundary restricted. To achieve some modification to `ComputeMaterialThread`,
`ComputeUserObjectThread` and to `FEProblemBase` have been necessary.
