# MultiAppUserObjectTransfer

!syntax description /Transfers/MultiAppUserObjectTransfer

## Description

MultiAppUserObjectTransfer transfers information from a UserObject in the parent/sub application to an AuxVariable in the sub/parent application based on the direction of transfer (to_multiapp/from_multiApp).

The transfer can be restricted to a subdomain using the [!param](/Transfers/MultiAppUserObjectTransfer/block) parameter or a boundary using the [!param](/Transfers/MultiAppUserObjectTransfer/boundary) parameter.

## To MultiApp

If the AuxVariable in the sub applications is a nodal AuxVariable, for each node in the sub application the value at the corresponding position in the parent application is queried from the parent UserObject, and this value is set to the AuxVariable at that node. A similar approach is followed for the elemental AuxVariable but with the centroid of the  element instead of nodal position.

## From MultiApp

For nodal AuxVariable in the parent application, it is first determined whether the node is contained within the bounding box of the sub application. If the parent node lies within a sub application's bounding box, the value of the sub application UserObject at that location is transferred to the parent AuxVariable. A similar approach is followed for the elemental AuxVariable but with the centroid of the parent element instead of nodal position.

When `all_parent_nodes_contained_in_sub_app` option is set to true, an error is generated if the parent node/element does not lie within the bounding boxes of any of the sub applications. An error is also generated if the parent node/element lies within the bounding boxes of 2 or more sub applications.

!syntax parameters /Transfers/MultiAppUserObjectTransfer

!syntax inputs /Transfers/MultiAppUserObjectTransfer

!syntax children /Transfers/MultiAppUserObjectTransfer

!bibtex bibliography
