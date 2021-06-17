# PorousFlowAddBCAction

This action creates `PorousFlow`-specific boundary conditions. It allow developers to create
user friendly syntax for adding more complicated boundary conditions using the following structure:

```
[PorousFlow]
  [BCs]
    [my_bc]
      type = MyUserFriendlyBC
      parameters
    []
  []
[]
```

To create a syntax like this, developers have to build a proxy class called `MyUserFriendlyBC`.
This class inherits from `MooseObject` and will register its own parameters as usual.

Then, in `PorousFlowAddBCAction::act()` developers check the type of a constructed object and if
it matches their type, they build the underlying boundary conditions objects using the MOOSE C++
inferface.

An example of this approach can be seen in (PorousFlowSinkBC)[PorousFlowSinkBC.md] which is
the user-friendly proxy class and the way it is setup can be seen in `PorousFlowAddBCAction.C`.
