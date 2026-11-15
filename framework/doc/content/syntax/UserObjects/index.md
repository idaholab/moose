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
  an +ElementUserObject+, +InternalSideUserObject+, +SideUserObject+, and +InterfaceUserObject+.
  See [DomainUserObject side execution](#domain-user-object-side-execution) for how sides are
  dispatched to these operations.
- +MortarUserObject+: "execute" is called for each mortar segment element corresponding to the
  secondary-primary mortar interface pairing on each execution flag

## DomainUserObject side execution id=domain-user-object-side-execution

In MOOSE, a boundary is an ID attached to element sides (a side set) or nodes (a node set). In the
context of side operations, a boundary ID on an element side does not imply that the side has no
neighbor: a side with a neighboring element may also have one or more boundary IDs. A
`DomainUserObject` processes these two properties independently:

- A side with a neighbor undergoes internal-side evaluation when both elements are within the
  object's block restriction.
- Each boundary ID on a side is dispatched according to `boundaries` and `interface_boundaries`.
  An ID selected by `boundaries` undergoes boundary evaluation, whether or not the side has a
  neighbor. An ID selected by `interface_boundaries` undergoes interface evaluation, requires a
  neighbor, and makes face data (notably variable values and material properties from both the
  primary and neighbor sides) available.

Consequently, a side with a neighbor and a boundary ID undergoes internal-side evaluation together
with either boundary evaluation or interface evaluation. The boundary-ID operations are mutually
exclusive for a given ID, but neither is mutually exclusive with internal-side evaluation.

The `boundaries` parameter has the following selection behavior:

- If unset, every boundary ID not listed in `interface_boundaries` undergoes boundary evaluation.
- If set to an empty list, no boundary evaluations are performed.
- If set to a list, only the listed IDs undergo boundary evaluation.

If `interface_boundaries` is unset or empty, interface evaluations are not performed. The same
boundary ID cannot be listed in both parameters.

The following cases illustrate the resulting side operations:

1. A side with no neighbor and the boundary ID `wall` undergoes boundary evaluation when `wall` is
   selected by `boundaries`. Selecting `wall` with `interface_boundaries` instead produces an error
   because an interface requires a neighbor. The side does not undergo internal-side evaluation.
2. A side with a neighbor and no boundary ID undergoes only internal-side evaluation when both
   elements are within the object's block restriction. The `boundaries` and `interface_boundaries`
   parameters do not disable this operation.
3. A side with a neighbor and the boundary ID `crack` undergoes internal-side evaluation and
   boundary evaluation when `crack` is selected by `boundaries`. It undergoes internal-side
   evaluation and interface evaluation when `crack` is selected by `interface_boundaries`.
4. If a side with a neighbor has both `crack` and `marked`, setting `boundaries = marked` and
   `interface_boundaries = crack` executes all three operations: boundary evaluation for `marked`,
   interface evaluation for `crack`, and internal-side evaluation because the side has a neighbor.
5. An internal side between an element within the object's block restriction and a neighboring
   element outside it does not undergo internal-side evaluation. If the side has a boundary ID
   selected by `boundaries`, it undergoes boundary evaluation for the element within the block
   restriction. If the boundary ID is selected by `interface_boundaries`, it undergoes interface
   evaluation, making face data from both the primary element and the external neighbor available.

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

10. `threadJoin` and `finalize` are called for `ThreadedGeneralUserObject`s.

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
