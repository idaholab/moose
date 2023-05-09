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
   on each execution flag. If the boundary is internal within the mesh, only variables, material
   properties, etc. at the primal side of the boundary are available.
- +InterfaceUserObject+: "execute" is called for each +side+, that is on an internal boundary,
   within the mesh on each execution flag. Variables, material properties, etc. at both the primary
   and the secondary side of the internal boundary are available.
- +DomainUserObject+: this object is capable of executing all the operations of
  a +ElementUserObject+, +InternalSideUserObject+, +SideUserObject+ and +InterfaceUserObject+.
- +MortarUserObject+: "execute" is called for each mortar segment element corresponding to the
  secondary-primary mortar interface pairing on each execution flag

# Execution order

Within an execution stage set by the [`execute_on`](SetupInterface.md) parameter, user objects are executed in
the following order:

1. `residualSetup` / `jacobianSetup`

   If the current `execute_on` flag is either `EXEC_LINEAR` or `EXEC_NONLINEAR` the `residualSetup`
   and `jacobianSetup` are called respectively in the following order

   1. for objects derived from `ElementUserObject`,  `SideUserObject`, `InternalSideUserObject`
      `InterfaceUserObject`, and  `DomainUserObject`.
   2. for objects derived from `NodalUserObject`.
   3. for objects derived from `ThreadedGeneralUserObject`.
   4. for objects derived from `GeneralUserObject`.

2. `initialize` is called for objects derived from `ElementUserObject`,  `SideUserObject`,
   `InternalSideUserObject` `InterfaceUserObject`, and  `DomainUserObject` in that order.

3. All active local elements are iterated over and objects derived from `ElementUserObject`,
   `SideUserObject`, `InternalSideUserObject` `InterfaceUserObject`, and  `DomainUserObject` are
   executed on elements, boundaries attached to the element, internal sides between elements, and on subdomain changes, in that order.
   The order within each type group is determined through dependency resolution.

4. `threadJoin` and `finalize` are called in the following order

   1. for objects derived from `SideUserObject`
   2. for objects derived from `InternalSideUserObject`
   3. for objects derived from `InterfaceUserObject`
   4. for objects derived from `ElementUserObject`
   5. for objects derived from `DomainUserObject`

5. `initialize` is called for objects derived from `NodalUserObject`.

6. `NodalUserObject` are looped over all local nodes and executed on each node.
   On each node, the order is determined through dependency resolution.

7. `threadJoin` and `finalize` are called for `NodalUserObject`s.

8. `initialize` is called for objects derived from `ThreadedGeneralUserObject`.

9. `execute` is called for objects derived from `ThreadedGeneralUserObject` in a threaded way.

10. `threadJoin` and `finalize` ar called for `ThreadedGeneralUserObject`s.

12. `initialize`, `execute`, and `finalize` are called in that order for each `GeneralUserObject` (which in turn are ordered through dependency resolution within the set of applicable `GeneralUserObject`s).

For additional control over the user object execution order every user object has a `execution_order_group`
parameter of type integer. This parameter can be used to force multiple execution rounds of the
above order, so it effectively supersedes all the ordering described above. All objects with the same `execution_order_group` parameter value are executed in the
same round, and groups with a lower number are executed first. The default `execution_order_group`
is 0 (zero). Negative values can be specified to force user object execution *before* the default group, and
positive values can be uses to create execution groups that run *after* the default group. Execution
order groups apply to all `execute_on` flags specified for a given object.

## Restartable Data

Since UserObjects often create and store large data structures, the developer of a UserObject
should consider whether or not those data structures needs to be declared as "restartable".
Knowing whether or not a data structure should be declared restartable is straight-forward:
If the information in a data structure is expected to persist across time steps, it +must+
be declared as restartable. Conversely, if the data in a data structure is recomputed
at each invocation of the UserObject, no action is necessary. See [restart_recover.md](restart_recover.md optional=true)
for more information.
