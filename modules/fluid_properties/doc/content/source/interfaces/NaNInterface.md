# NaNInterface

For some objects, it is sometimes desirable to have the choice of whether
to use a signaling NaN or a quiet NaN. For example, for some simulations,
one may want to run in debug mode but not crash on a NaN from a certain
object. This class adds a parameter to control this behavior and an interface
for getting the corresponding NaN.
