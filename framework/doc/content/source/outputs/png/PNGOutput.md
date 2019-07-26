# PNGOutput (Outputs)

The PNGOutput object is the class for writing png files from the input files.

Following is an example of all the different input variables that can be used.

```
[Outputs]
  [png]
    type = PNGOutput
    transparent = true    #indicates whether the background will be transparent
    resolution = 50       #number of pixels in longest direction
    color = RWB           #indicates which color scheme to use (required)
    variable = u          #selection of the variable to use to create the image
    max = 6.4             #the max value in var_name needed for scaling
    min = -2              #the min value in var_name needed for scaling
    out_bounds_shade = .5 #value from 0-1 indicating shade to use as background
    transparency = 1      #value from 0(transparent)-1(opaque) for image.
  []
[]
```
