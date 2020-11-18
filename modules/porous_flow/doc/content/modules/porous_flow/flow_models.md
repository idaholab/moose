# Available flow models

The PorousFlow module is designed to enable simulations with any number of fluid
components in any number of fluid phases. Several specialized flow models are available
in PorousFlow to simulate some common scenarios.

General formulations are provided for immiscible single and multiphase models, including hysteresis:

- [Single phase](singlephase.md)
- [Multiphase](multiphase.md)

Specialized formulations for miscible two-phase flow are also provided, that use
a [persistent](persistent_variables.md) set of primary variables. Multiple fluid
components are handled using a [compositional flash](compositional_flash.md) to
calculate the partitioning of fluid components amongst fluid phases:

- [Water/steam](water_vapor.md)
- [Water and non-condensable gas](waterncg.md)
- [Brine and CO$_2$](brineco2.md)
