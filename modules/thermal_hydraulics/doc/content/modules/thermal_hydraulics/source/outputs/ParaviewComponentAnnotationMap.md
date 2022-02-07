# ParaviewComponentAnnotationMap

Use this outputter to produce an annotated color map for paraview (works with version 5.8+)

## Usage

Add the following block into the `[Outputs]` block in your input file:

```
[map]
  type = ParaviewComponentAnnotationMap
[]
```

This will produce a file named `<input_file>_map.json`.

Import this file into paraview via the +Color Map Editor+ window for the `vtkBlockColors` variable.
You should see a component name associated with each color. See example below:

!media thermal_hydraulics/misc/paraview_annotated_color_map.png
       id=paraview-annotated-color-map
       style=width:80%;margin-right:auto;margin-left:auto;
       caption=Example of annotated color map for component blocks

!syntax parameters /Outputs/ParaviewComponentAnnotationMap groups=Required

!syntax inputs /Outputs/ParaviewComponentAnnotationMap

!syntax children /Outputs/ParaviewComponentAnnotationMap
