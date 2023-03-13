# FunctionElementIntegralUserObject

!syntax description /UserObjects/FunctionElementIntegralUserObject

The `FunctionElementIntegralUserObject` is mostly used to apply an integration operation
to a function when creating other user objects by inheriting from both this class, for
the integration operation, and another class that defines how to interact with the mesh to
create custom spatial integration user objects, such as the
[FunctionLayeredIntegral](/userobjects/FunctionLayeredIntegral.md).

Used on its own, this user object performs a similar role as the [FunctionElementIntegral.md],
except that it does not set up the default output to console and CSV. Using the postprocessor is the
preferred way of performing this spatial function integration.

!syntax parameters /UserObjects/FunctionElementIntegralUserObject

!syntax inputs /UserObjects/FunctionElementIntegralUserObject

!syntax children /UserObjects/FunctionElementIntegralUserObject
