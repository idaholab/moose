# PostprocessorSpatialUserObject

The `PostprocessorSpatialUserObject` stores a postprocessor value inside a user object such that it gains a spatial position and can be used via `spatialValue()` API.
This can be useful in a multi-app setup when we need to transfer postprocessor values from sub apps and have them live at a position where
the sub-app exists.

## Example of a Multi-App Setup

!listing test/tests/userobjects/postprocessor_spatial_user_object/parent.i

!listing test/tests/userobjects/postprocessor_spatial_user_object/sub.i


!syntax parameters /UserObjects/PostprocessorSpatialUserObject

!syntax inputs /UserObjects/PostprocessorSpatialUserObject

!syntax children /UserObjects/PostprocessorSpatialUserObject
