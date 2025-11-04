# ActivateElementsUserObjectBase

The `ActivateElementsUserObjectBase` class is for activating (adding) an element by moving the element from an "inactive" subdomain to the "active" subdomain. There are two metrics for activating an element, i.e., by +function path+ and by +coupled variable value+, which are implemented in the classes [ActivateElementsByPath](/ActivateElementsByPath.md) and [ActivateElementsCoupled](/ActivateElementsCoupled.md), respectively.

This mesh modifier updates the boundary information due to the addition of elements across subdomains. User can save this boundary information by passing the changed boundary name to  `expand_boundary_name` in the input block.  Note that current implementation does not de-activate an element once the element is activated.
