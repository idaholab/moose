# ElementIntegralVariableUserObject

!syntax description /UserObjects/ElementIntegralVariableUserObject

The `ElementIntegralVariableUserObject` is mostly used to apply an integration operation
to a variable when creating other user objects by either:
- specializing user object class templates with this integration operation, for example the
  [NearestPointLayeredIntegral.md] specializes the `NearestPointBase` with this class and the
  [LayeredIntegral.md] to perform nearest point layered integration.
- inheriting both this class, for the integration operation, and another class that defines how
  to interact with the mesh to create a custom spatial integration user object, such as the
  [LayeredIntegral.md].


Used on its own, this user object performs a similar role as the [ElementIntegralVariablePostprocessor.md],
except that it does not set up the default output to console and CSV. Using the postprocessor is the
preferred way of performing this spatial variable integration.

!syntax parameters /UserObjects/ElementIntegralVariableUserObject

!syntax inputs /UserObjects/ElementIntegralVariableUserObject

!syntax children /UserObjects/ElementIntegralVariableUserObject
