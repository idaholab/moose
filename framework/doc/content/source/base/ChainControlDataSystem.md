# ChainControlDataSystem

The `ChainControlDataSystem` manages [/ChainControlData.md] objects and provides
interfaces to do the following:

- Check if control data exists (and if it has a certain type).
- Get control data
- Declare control data

This object is owned by [/MooseApp.md], which provides a `getChainControlDataSystem()`
method to get it.
