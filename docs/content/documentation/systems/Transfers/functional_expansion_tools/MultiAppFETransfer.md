# MultiAppFETransfer
!syntax description /Transfers/MultiAppFETransfer

This `Transfer` transfers the coefficients from an FE-generating object (such as the `FE...UserObject`s) to a FE-utilizing object (such as a `FunctionSeries`). It leverages `MutableCoefficientsInterface`, from which all of these objects must subclass, to perform the transfers.

It searches the associated **MultiApp** objects for the named objects to perform the transfers. It also ensures that each is a subclass of `MutableCoefficientsInterface`.

Note: `MultiAppFETransfer` is actually a typedef of `MultiAppMutableCoefficientsTransfer`.


!syntax parameters /Transfers/MultiAppFETransfer

!syntax inputs /Transfers/MultiAppFETransfer

!syntax children /Transfers/MultiAppFETransfer
