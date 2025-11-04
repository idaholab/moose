# Paraview Basics

Paraview can read a number of different file formats, including the Exodus
format, which is the primary output format for MOOSE.

## Paraview Workspace

The Paraview workspace consists of the following basic areas:

- +Header rows+: These contain a number of different buttons and dropdowns for
  various operations such as opening files, playing transients, controlling
  views, and creating filters.
- +Pipeline browser+ (Located on the left, directly under the header rows):
  This shows the loaded files and the pipeline of filters applied to those files.
- +Properties Pane+ (Located underneath the pipeline browser): This shows a
  number of different configurable properties that apply to the selected
  pipeline element.
- +View pane+ (Located in the main, large area): This contains the views for
  the output, which could be renderings or line plots, for example.

## Opening a File

To open a file, do the following:

- `File->Open`
- Select the file and click "OK".
- In the properties pane, select which variables and blocks you would like.
  Unless you'd like to specifically exclude some variables, it is safe just to
  select all of them (in Paraview you can select or unselect all checkboxes in
  a group by clicking the checkbox next to the group name). Blocks should all
  be selected by default, but you can choose not to load some blocks if they
  are not of interest.

!alert note title=Starting Paraview from the command line
Paraview can optionally be started on the command line, which has the advantage
that the file browser will start from your current working directory, which is
typically where your output files of interest are located. Search for the path
to the executable on your file system, for example, `/path/to/paraview`. Then
rather than specify this full path every time, you can create an alias: `alias
paraview='/Applications/paraview.app/Contents/MacOS/paraview'`. You could then
put this in your `~/.bashrc` or equivalent file to have the alias created every
terminal session.

!alert note title=Applying changes in Paraview
Anytime a change is made to the pipeline (opening a file, adding a filter, changing
filter properties, etc.), those changes must be applied by clicking the "Apply"
button in the properties pane. If you want changes to automatically be applied,
you can click the checkbox in `Paraview->Preferences`.

## Viewing 1-D Data

To view 1-D data, do the following:

- (Optional) Before viewing any 1-D data, it can be helpful to unload any blocks
  corresponding to multi-D solutions (click the file name in the pipeline browser and
  deselect the blocks in the properties pane).
- Create a "Plot Over Line" filter: `Filters->Data Analysis->Plot Over Line`.
- Select the begin and end points for the line: in the Properties pane, you'll
  see "Point1" and "Point2" which are points in 3-D space that define the line
  to be sampled. As a note, this could also be used to plot a line through a
  multi-D mesh. If you have only your 1-D meshes, and they are collinear, then
  the default points will be the ones you want. In general, it selects points to
  get the smallest bounding box. This is the reason for the optional first step.
- Change the sampling resolution: in the Properties pane, there is a box "Resolution",
  which is the number of points Paraview will take along the defined line.
  It is recommended to use something quite high, say 1000; otherwise, you may
  see plotting artifacts. Note that with a high resolution, you will see that
  the solution is piecewise constant. You could select a resolution that
  more-or-less matches your number of elements, but you'll probably see a kink
  in the solution somewhere and falsely think there is a kink in the solution,
  while in fact it just happens to be where two sampling points finally hit the same element.
- Click "Apply".
- Close the "Render View" if you do not want it.
- Select the variables to view: with the "Plot Over Line" filter in the pipeline
  selected, select in the properties pane the variables you wish to view.
- Created additional plots by clicking the "Split Horizontal" or "Split Vertical"
  buttons in the upper right corner of a line chart view. Then with a view selected,
  make sure to activate the eyeball icon next to your "Plot Over Line" filter in
  the pipeline browser.
- Use the green arrows in the header rows to play the transient or step through
  one time step at a time.

## Viewing Multi-D Data

To view multi-D data, do the following:

- (Optional) Before viewing any multi-D data, it can be helpful to unload any blocks
  that do not have the variables of interest. For example, blocks corresponding
  to 1-D channels will not have the variable `T_solid` like the blocks corresponding
  to 2-D heat structures. This prevents the scale on color plots from being
  auto-scaled incorrectly, since a non-existent variable on a block will have
  the value 0 in Paraview.
- Select the variable of interest from the dropdown in the header rows (which
  typically has "vtkBlockColors" selected by default).
- Zoom level can be controlled with the scroll wheel.
- Use the green arrows in the header rows to play the transient or step through
  one time step at a time.
- Rescale if you wish by using buttons in the header row; there are options for
  scaling to custom values or to visible values for the current time step.

!alert note title=Stretching the coordinate system
Some blocks have very small/large aspect ratios, so it can be useful to stretch
the coordinate system. After loading a file, with the file selected in the
pipeline browser, go to the "Transformation" section near the bottom of the
properties pane and use the "Scale" boxes for the x, y, and z directions as
needed.

## Saving a Paraview State

A Paraview state can be saved with `File->Save State`. Then you can open up
Paraview and automatically load all of the exact same operations you did before
with `File->Load State` (and selecting the `.pvsm` file you created).
There are a number of instances where saving a Paraview state can be useful,
for example:

- when running an input file multiple times, and
- when viewing output while an input file is still running and want to see an update
  (in this case, you'll want to make sure to reset Paraview first before loading
  the state by doing `Edit->Reset Session`).

If you are opening up Paraview from the command line, you can specify the state
file as an argument:

```
paraview --state=my_state_file.pvsm
```
