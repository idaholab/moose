# UserObject System

The UserObject system is developing and running custom algorithms that may not fit well within
any other system in MOOSE. Examples include complex calculations that may result values that
don't associate in a one to one manner with elements, nodes, or sides. Perhaps your user object
produces values based on height in your domain or based on groups of related elements (not
necessarily associated with subdomains or other static mesh features). Often these calculations
may result in custom data structures that can be managed by the developer.

The UserObject system is the basis for the [Postprocessor](syntax/Postprocessors/index.md) system.
The only difference being that Postprocessors have an additional interface for returning values
and corresponding managed storage within the framework for retrieving those values through
well-defined interfaces \([PostprocessorInterface.md]\).

## User-defined APIs

One benefit of the UserObject system is that it is designed with user-defined APIs in mind.
UserObjects can and should define additional `public` methods that may be used to retrieve
data computed by the UserObject. The [UserObjectInterface.md] is templated so that an object
that depends on a specific type of UserObject may retrieve that specific type of UserObject
and use the interface without the use of dynamic casts.

## Types of UserObjects

- +GeneralUserObject+: "execute" is called once on each execution flag.
- +NodalUserObject+: "execute" is called for each +node+ within the mesh on each execution flag.
- +ElementalUserObject+: "execute" is called for each +element+ within the mesh on each execution
   flag.
- +InternalSideUserObject+: "execute" is called for each +side+, that is not on a boundary,
   within the mesh on each execution flag.
- +SideUserObject+: "execute" is called for each +side+, that is on a boundary, within the mesh
   on each execution flag.
- +DomainUserObject+: this object is capable of executing all the operations of
  a +ElementUserObject+, +InternalSideUserObject+, and +SideUserObject+

## Restartable Data

Since UserObjects often create and store large data structures, the developer of a UserObject
should consider whether or not those data structures needs to be declared as "restartable".
Knowing whether or not a data structure should be declared restartable is straight-forward:
If the information in a data structure is expected to persist across time steps, it +must+
be declared as restartable. Conversely, if the data in a data structure is recomputed
at each invocation of the UserObject, no action is necessary. See [restart_recover.md](restart_recover.md optional=true)
for more information.
