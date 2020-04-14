# UserObjectInterface

The UserObjectInterface defines the methods used for retrieving const references to the specific types of
UserObjects. This is done with templated methods so that use of the UserObject does not require a dynamic
cast. Many objects in MOOSE support the retrieval of these references so that custom UserObject APIs may
be used in calculations. UserObjects are generally executed after most other systems in MOOSE so these
values are often lagged when being used in another calculation.

## End Use API

Most objects in MOOSE will retrieve UserObjects through special macro functions found here:

!listing /UserObjectInterface.h start=doco-user-object-interface-begin end=doco-user-object-interface-end include-start=false

Typical usage looks like this:

!listing ElementUOAux.C start=doco-get-user-object-begin end=doco-get-user-object-end include-start=false
