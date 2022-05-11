# MultiAppConservativeTransfer

It is essential to preserve a physics quantity during the transfer process for specific applications. For instance, neutron calculations' total power needs to be preserved when the power density is transferred to thermal calculations. That was implemented in the conservative transfer. The transfer is performed in the following steps:

- Users get a solution in the source app ready and calculate to-be-preserved physics quantity based on the solution.  The physics quantity is calculated using a postprocessor where users determine how they want to compute this quantity.

- The solution is transferred to the target domain. A new physics quantity is calculated using the target solution. Again, that calculation is implemented as a postprocessor, where users should compute that according to the physics.

- The target solution will be adjusted according to the source physics quantity and the target physics quantity.

- The conservative quantity will be calculated again based on the adjusted target solution. The value is often printed on the screen for users' convenience.

The main picture of the conservative transfer can be explained mathematically as follows

$Q_1(u_1) = Q_2(u_2)$,

where $Q_1$ is a function used to compute a quantity of interest on the source domain and $u_1$
is a source physics field. $Q_1$ is implemented as a postprocessor. $Q_2$ and $u_2$ are corresponding
counterparts on the target side.  We emphasize that users are free to implement any postprocessor for $Q_i$ according to the physics of interest.

## Example Input File Syntax

The following examples demonstrate the use of the MultiAppConservativeTransfer for transferring
solution.


!listing multiapp_conservative_transfer/parent_conservative_transfer.i block=Transfers caption=Example use of MultiAppConservativeTransfer for transferring data conservatively.


!listing multiapp_conservative_transfer/parent_conservative_transfer.i block=Postprocessors caption=From postprocessor.

!listing multiapp_conservative_transfer/sub_conservative_transfer.i block=Postprocessors caption=To postprocessor.  Note that `to postprocessor` need to be executed on `transfer`.
