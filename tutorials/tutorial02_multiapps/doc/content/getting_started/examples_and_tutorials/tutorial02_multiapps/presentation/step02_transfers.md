# Step02 Transfers

!---

## Transfers Overview

Transfers are how you move information up and down the MultiApp hierachy

There are three different places Transfers can read information and deposit information:

- AuxiliaryVariable fields
- Postprocessors
- UserObjects

A few of the most-used Transfers:

- MeshFunction: Interpolate a field from one domain to another
- NearestNode: Move field data by matching nodes/centroids
- Postprocessor: Move PP data from one app to another
- UserObject: Evaluate a "spatial" UO in one app at the other app's nodes/centroids and deposit the information in an AuxiliaryVariable field

Most important: by having MOOSE move data and fill fields... apps don't need to know or care where the data came from or how it got there!

!---

## MultiAppMeshFunctionTransfer

!row!
!col! width=80%
Called "MeshFunction" because it makes a field (Solution or Aux) into a spatial "function" that can be evaluated anywhere within the domain.  This is then used to sample that field at the nodes/centroids of the *receiving* app's mesh in order to populate an Auxiliary Variable field.

Required parameters:

- `direction`: Like all Transfers, this is either `to_multiapp` or `from_multiapp`
- `multi_app`: Which MultiApp to interact with
- `source_variable`: The variable to read from
- `variable`: The Auxiliary variable to write to

Can be made to "conserve" a Postprocessor quantity.

!col-end!

!col width=20%
!media images/transfers_01_meshfunction.png
       style=width:100%;margin-left:auto;margin-right:auto;display:block;box-shadow:none;

!row-end!

!---

## Master Input file

!listing step02_transfers/01_parent_meshfunction.i
         caption=01_parent_meshfunction.i

!---

## Sub-App Input file

!listing step02_transfers/01_sub_meshfunction.i
         caption=01_sub_meshfunction.i

!---

## Run 01_parent_meshfunction.i

- Note when the Transfers happen
- Look at the output files
- Try changing the sub-app position to `0.5 0 0` to see what happens

!---

## MultiAppNearestNodeTransfer

Sometimes interpolation is too costly, or doesn't make sense.  For instance, a "2D" calculation in the x,y plane that represents an "infinite" volume in the z direction may need to be coupled to 3D calculations that cover that same space.  In that case, all nodes/elements within the same x,y "column" should receive the same value when transferring from the 2D calculation to the 3D calculation.

This is easily achieved with the NearestNodeTransfer.

The idea is that each node (or element centroid for CONSTANT/MONOMIAL fields) in the receiving mesh is paired with its nearest match in the sending mesh.  Data can then be easily moved from the sending mesh to the receiving mesh using these pairs.

This may seem rudimentary - but it is extremely handy...

!---

## NearestNode Setup

!row!
!col! width=70%
To explore the NearestNodeTransfer we will create a situation where a 2D, 1x1 square will be sending and receiving to three 3D "columns" sticking out from it, as shown to the right.

!listing step02_transfers/02_parent_nearestnode.i
	 block=Transfers
         caption=02_parent_nearestnode.i

!col-end!

!col width=30%
!media images/transfers_02_geometry.png
       style=width:100%;margin-left:auto;margin-right:auto;display:block;

!row-end!

!---

## Run 02_parent_nearestnode.i

- Open all 4 outputs
- Step through time and watch the values change
- How do the values in the columns relate to the values in the sub-apps?

!---

## MultiAppUserObjectTransfer

Often it's not "fields" that need to be moved - but spatially homogenized values.  For instance, if a 1D simulation representing fluid flow through a pipe is going through a 3D domain which is generating heat - then it may be advantageous to integrate the heat generated around the pipe within the 3D domain and transfer that to the pipe simulation.

This situation is handled straighforwardly by UserObject Transfers.  Remember that Postprocessors are a specialization of UserObjects: they only compute a single value.  UserObjects themselves can then be thought of as Postprocessors that can compute much more than a single value.  Within MOOSE there is a special designation for UserObjects that hold data with some spatial correlation: they are "Spatial UserObjects"... and they have the ability to be evaluated at any point in space (and sometimes time!).

Some Spatial UserObjects that could be useful to Transfer:

- (NearestPoint)LayeredAverage: Computes the average of a field in "layers" going in a direction.  The "NearestPoint" version can create multiple LayeredAverages from the elements surrounding given points.
- (NearestPoint)LayeredIntegral: Similar
- (NearestPoint)LayeredSideAverage: Similar
- SolutionUserObject: Presents a solution from a file as a spatial UserObject

!---

## Run 03_parent_uot.i

!listing step02_transfers/03_parent_uot.i
         caption=03_parent_uot.i

This example is similar to the last one - except we've made the master domain 3D as well.  The idea is to integrate the field from the master in the vicinity of the sub-apps and transfer that to each sub-app.  In the reverse, the sub-app field is averaged in layers going up the column and those average values are transferred back to the Master.

!---

## Multiscale Transfers

!row!
!col! width=80%
When the sub-app domain represents an infinitesimally small portion of the Master's domain - a different type of Transfer is needed.  For instance, if the Master domain is 1m x 1m and the sub-app domains are 1nm x 1nm, then there is no point in trying to "interpolate" a field from the Master domain.  Effectively, the sub-apps lie at *points* inside the master domain.

The final class of Transfers, those moving scalar values, play a role here.  "Sampling" Transfers such as `VariableValueSampleTansfer` will automatically evaluate a single value from the Master domain at the sub-app position and move it to the sub-app.  In the reverse direction, Postprocessor Transfers move homogenized values from the sub-apps to the Master app.

!col-end!

!col width=20%
!media images/transfers_04_multiscale.png
       style=width:100%;margin-left:auto;margin-right:auto;display:block;box-shadow:none;

!row-end!

!---

## Run 04_parent_multiscale

!listing step02_transfers/04_parent_multiscale.i
         caption=04_parent_multiscale.i

Here a `VariableValueSampleTransfer` is used to get values from the Master domain to the sub-apps.  In the reverse a `PostprocessorInterpolationTransfer` works to take values computed in the micro-simulations and then interpolate between them to create a smooth field.
