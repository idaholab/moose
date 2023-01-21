# Visualizing Results

!---

## Output Files

Preferred output for MOOSE applications is
ExodusII [!citep](exodus:96) binary format. Several options exist for visualizing ExodusII files:

!style! fontsize=85%
-   ParaView

  -   Open-source general visualization tool

-   Ensight

  -   Commercial general visualization tool

-   Peacock

  -   MOOSE GUI has integrated postprocessor
  -   Live update of results while model is running

-   Blot

  -   Command-line visualization tool
  -   Part of SEACAS suite of codes for working with Exodus files
  -   Easily scripted, useful for generating x-y plots

-   Patran

  -   Commercial pre- and post-processor, requires Exodus plugin

-   VisIt

  -   Open-source general visualization tool

!style-end!

!---

## [ParaView](https://www.paraview.org/download/)

!row!

!col! width=60%
!col width=60%

-   Open-source GUI-based visualization tool

-   Provides readers for many data formats, including Exodus

-   Intended for visualization of very large datasets

    -   Remote parallel rendering

    -   Some behavior of the user interface driven by that emphasis.

    -   Strong preference toward loading minimal data into memory.

-   Thin GUI layer on top of VTK open-source visualization toolkit
    (Kitware).

    -   Same software used for displaying graphics in Cubit

-   Brief usage tutorial provided in the following slides

!col-end!

!col! width=2%
!col width=2%

$~$

!col-end!

!col! width=38%
!col width=38%

$~$

$~$

!style halign=center
!media paraview/pv_splash.png


!col-end!

!row-end!

!---

## Setting an Alias

- To allow opening of ExodusII files from the command line, adding an
  alias can be useful.

- Open your `.bash_profile` on Mac or `.bashrc` on Linux and add an alias to the
  ParaView executable.  For example, for ParaView version 5.6.0 on a Mac:

```bash
alias paraview="/Applications/ParaView-5.6.0.app/Contents/MacOS/paraview"
```

- This allows you to open a specific ExodusII file via:

```bash
paraview my_awesome_file.e
```

!---

## Brief ParaView Tutorial

We will use a BISON example problem to demonstrate functionality within ParaView.

!style halign=center
Additional tutorials for ParaView can be found [here](https://www.paraview.org/download/).

!---

## Other Postprocessing Options

BISON can output global scalar quantities of interest to a CSV file for further
post-processing using Postprocessors.

- The CSV can be imported into Excel.

- The CSV can be read into Python for journal quality plots through [matplotlib](https://matplotlib.org/).

BISON can also output global scalar quantities of interest along a specified line or boundary
using VectorPostprocessors.
