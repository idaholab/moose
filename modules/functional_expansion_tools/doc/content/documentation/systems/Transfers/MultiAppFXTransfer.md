# MultiApp FX Transfer

!syntax description /Transfers/MultiAppFXTransfer

## Description

This `Transfer` transfers the coefficients from an FX-generating object (such as the `FX...UserObject`s) to a FX-utilizing object (such as a `FunctionSeries`). It leverages `MutableCoefficientsInterface`, from which all of these objects must subclass, to perform the transfers.

It searches the associated **MultiApp** objects for the named objects to perform the transfers. It also ensures that each is a subclass of `MutableCoefficientsInterface`.

## Example Input File Syntax

!listing modules/functional_expansion_tools/examples/1D_volumetric_Cartesian/main.i block=Transfers id=input caption=Example use of MultiAppFXTransfer

!syntax parameters /Transfers/MultiAppFXTransfer

!syntax inputs /Transfers/MultiAppFXTransfer

!syntax children /Transfers/MultiAppFXTransfer
