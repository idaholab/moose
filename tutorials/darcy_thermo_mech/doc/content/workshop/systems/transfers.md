# Transfer System

A system to move data to and from the "parent application" and "sub-applications" of a MultiApp.

!---

Transferred data typically is handled by the Auxiliary and Postprocessor systems.

The data on the receiving application should couple to these values in the normal way and
each sub-application should be able to solve on its own

!---

## Framework Transfer Classes

- `MultiAppCloneReporterTransfer`: Declare and transfer reporter data from sub-application(s) to main application
- `MultiAppCopyTransfer`: Copies variables (nonlinear and auxiliary) between multiapps that have identical meshes
- `MultiAppInterpolationTransfer`: Transfers variables using mesh interpolation
- `MultiAppMeshFunctionTransfer`: Transfers variables from sub-applications
  using a parent application mesh-function object
- `MultiAppNearestNodeTransfer`: Transfer the value to the target domain from the nearest node in the source domain
- `MultiAppPostprocessorInterpolationTransfer`: Transfer postprocessor data from
  sub-application into field data on the master application
- `MultiAppPostprocessorToAuxScalarTransfer`: Transfers from a postprocessor to a scalar auxiliary variable
- `MultiAppPostprocessorTransfer`: Transfers postprocessor data between the master application and sub-application(s)
- `MultiAppProjectionTransfer`: Perform a L2 projection between a master and sub-application mesh of a field variable
- `MultiAppReporterTransfer`: Transfers reporter data between two applications
- `MultiAppScalarToAuxScalarTransfer`: Transfers data from a scalar variable to
  an auxiliary scalar variable from different applications
- `MultiAppUserObjectTransfer`: Samples a variable's value in the Parent app
  domain at the point where the MultiApp is and copies that value into a post-processor in the MultiApp
- `MultiAppVariableValueSamplePostprocessorTransfer`: Transfers field variable
  values from parent application to a child application postprocessor or in the
  reverse direction takes a postprocessor value from the nearest child
  application to populate a parent application's field variable degrees of freedom
- `MultiAppVariableValueSampleTransfer`: Transfers the value of a variable
  within the master application at each sub-application position and transfers
  the value to a field variable on the sub-application(s)
- `MultiAppVectorPostprocessorTransfer`: This transfer distributes the N values
                             of a VectorPostprocessor to Postprocessors located
                             in N sub-apps or collects Postprocessor values from
                             N sub-apps into a VectorPostprocessor


## Field Interpolation

- An "interpolation" `Transfer` should be used when the domains have some overlapping geometry.
- The source field is evaluated at the destination points (generally nodes or element centroids).
- The evaluations are then put into the receiving `AuxVariable` field named `variable`.
- All `MultiAppTransfers` take a `direction` parameter to specify the flow of information. Options are: `from_multiapp` or `to_multiapp`.

!listing exec_on_mismatch.i block=Transfers

!---

## UserObject Interpolation

- Many `UserObjects` compute spatially-varying data that is not associated directly with a mesh
- Any `UserObject` can override `Real spatialValue(Point &)` to provide a value given a point in space
- A `UserObjectTransfer` can sample this spatially-varying data from one app and put the values into an `AuxVariable` in another

!listing 3d_1d_parent.i block=Transfers

!---

## Postprocessor Transfer

A Postprocessor transfer allows a transfer of scalar values between applications

- When transferring to a `MultiApp`, the value can either be put into a `Postprocessor` value or can be put into a constant `AuxVariable` field
- When transferring from a `MultiApp` to the parent application, the value can be interpolated from all the sub-apps into an auxiliary field

!listing multiapp_postprocessor_transfer/parent.i block=Transfers
