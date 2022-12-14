# ConstantVectorPostprocessor

!syntax description /VectorPostprocessors/ConstantVectorPostprocessor

The constant data is specified with a vector names, and a vector of vectors of values for each name. The vectors of values are separated by semicolons.

A constant vector postprocessor is not generally used for output or postprocessing, but more to isolate part of the system using constant inputs. For example, if a subapp communicates with the main app by transferring a vector postprocessor, a `ConstantVectorPostprocessor` may be used to replace that transfer for debugging or parametric study purposes.

## Example input syntax

In this example, two `ConstantVectorPostprocessor` are used to specify three vectors `a b c`. They are tied to each vector postprocessor, so the two do not conflict despite their similar name.

!listing test/tests/transfers/multiapp_reporter_transfer/main.i block=VectorPostprocessors

!syntax parameters /VectorPostprocessors/ConstantVectorPostprocessor

!syntax inputs /VectorPostprocessors/ConstantVectorPostprocessor

!syntax children /VectorPostprocessors/ConstantVectorPostprocessor
