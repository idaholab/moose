# Getting Started

Chigger is generally a stand-alone package that is a part of the MOOSE repository, the source
code is located in the [python/chigger](https://github.com/idaholab/moose/tree/master/python/chigger) directory.

It is recommended that you install the correct MOOSE environment package (see the "Getting Started"
pages for more information). However, it is possible to manually setup your python environment to use
chigger, but detailing this setup is beyond the scope of the documentation presented here.

To be able to use chigger as a python package (i.e., `import chigger`) the `PYTHONPATH` environment
variable must be set as follows.

```bash
export PYTHONPATH=$PYTHONPATH:/your/path/to/moose/python
```

## Simple Example

The code in [chigger_simple] is a simple example that demonstrates the most basic functionality:
open and view a MOOSE exodus result file.

!listing simple/simple.py
         start=import chigger
         id=chigger_simple
         style=width:60%
         caption=Simple example for viewing exodus data with chigger.

To check that your setup is working correctly it is possible to run this example, using the following
steps.

1. Navigate to the chigger tests directory that contains the script.

   ```bash
   cd ~/projects/moose/python/chigger/tests/simple
   ```

2. Run the example from the command line.

   ```bash
   ./simple.py
   ```

Running the script will open a small window showing a 3D result file. By default it is not
possible to manipulate the result without activating it, the following section details how to begin
manipulating objects.

The chigger package is designed around creating custom scripts for producing images and movies of
results, in similar fashion as [matplotlib](https://matplotlib.org/) is designed for making plots.
As such, there is no graphical interface, if a GUI is desired [Peacock], [Paraview], or any other
visualization tool should be utilized.

It is possible to manipulate the various components of a chigger based visualization to aid in
building scripts as rapidly as possible, this is accomplished via keyboard shortcuts, terminal
based messages, and mouse interaction.

As listed in [general-keybindings], there are a set of general keyboard shortcuts that are available
at all times. Additionally, each component or result (i.e., a python `ChiggerResult` object) has
an additional set of keybindings that are available, but only when the result is activated.

!chigger keybindings object=chigger.observers.MainWindowObserver
                     id=general-keybindings
                     caption=General keybindings available interactive windows.

Continuing with the simple example, the following steps will demonstrate how to activate and
manipulate a result.

3. After selecting the open VTK window with the mouse, press the "h" key. This will print a
   a table, similar to what is below, that provides a list of the available keybindings.

   ```text
   $ ./simple.py
   General Keybindings:
         r: Select the next result object.
   shift-r: Select the previous result object.
         d: Clear selection(s).
         h: Display the help for this object.
   ```

   !alert note title=Keybindings only work after clicking the window twice.
   It may be required to click on the window twice for the operating system to focus on the window:
   the first click activates it at the operating system level, the second click, which must be within
   the visualization portion not on the border, activates the window with respect to VTK.

4. One of the general keybindings is the "r" key, pressing it will toggle the activation status of
   the various result(s) of a chigger visualization. In order to manipulate a result it must be
   activated. Pressing "r" for this simple example will activate the 3D exodus result, this will be
   evident because it will be highlighted with a bounding box; pressing "h" will now yield additional
   keybindings specific to the activated result, as shown below.

   ```text
   General Keybindings:
         r: Select the next result object.
   shift-r: Select the previous result object.
         d: Clear selection(s).
         h: Display the help for this object.

   Mug Data (ExodusResult) Keybindings:
         c: Display the camera settings for this object.
         a: Increase the alpha (opacity) by 1%.
   shift-a: Decrease the alpha (opacity) by 1%.
         m: Toggle through available colormaps.
   shift-m: Toggle through available colormaps, in reverse direction.
         t: Increase timestep by 1.
   shift-t: Decrease the timestep by 1.
   ```

With the result activated it is worth while to explore the available options, doing so will
cause the visualization to change. Any change made will show the associated option, with the
new value, on the command line. For example, pressing "shift-a", which changes the alpha value
(opacity), results in:

```text
Mug Data (ExodusResult):opacity=0.95
```

To make the change permanent for future executions of your script this value must be added to the
options for the associated object. This illustrates the purpose of he keybindings, which is
simply to provide a convenient way to help build up your script without re-executing.

## Options and Objects

The chigger package is designed around objects, such as readers, annotations, and results. Each
object is controlled by a set of key, value pairs called "options." These options are generally
set at construction time or by calling the `setOptions` method, for example:

```text
reader = chigger.exodus.ExodusReader('../input/mug_blocks_out.e', time=1.2)
reader.setOption(block=[76])
```

It is possible to display the available options for an active result object by pressing "o" or
"shift-o". The former produces a detailed list of all available options and the later produces
a string with a valid `setOptions` method call(s) for the non-default options.

There is a myriad of objects available within the chigger package. These objects are organized into
sub-packages, which are listed on the source page: [chigger/index.md].
