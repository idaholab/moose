# MultiAppInterpolationTransfer

The MultiAppInterpolationTransfer transfers the source variable to the nearest node on the
target mesh using mesh interpolation, including the ability to utilize the displaced
configuration for either or both the source and target.

## Example Syntax

!listing test/tests/transfers/multiapp_interpolation_transfer/fromsub_master.i start=[Transfers] end=elemental_fromsub footer=[]

!syntax parameters /Transfers/MultiAppInterpolationTransfer

!syntax inputs /Transfers/MultiAppInterpolationTransfer

!syntax children /Transfers/MultiAppInterpolationTransfer
