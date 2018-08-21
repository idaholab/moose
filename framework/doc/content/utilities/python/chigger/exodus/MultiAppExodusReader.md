# MultiAppExodusReader

As the name suggests, the `MultiAppExodusReader` is an extension of the [`ExodusReader`](ExodusReader)
object designed for using with the [MultiApps] system within [MOOSE].

## Reader Example

The `MultiAppExodusReader` a stand alone object does not provide any visualization abilities, but
may be useful for gathering information regarding the associated files. The following will produce
information for each of the supplied exodus files.

!listing exodus/multiappreader.py start=import

# Rendered Example

The [MultiApps] system is often used to multiple simulations across a spatial domain. The
`MultiAppExodusReader` allows for the outputs from such a simulation to be visualized using a
common renderer, thus the separate files act as a single entity.

The code in [multiapp-render] opens the output from a multiapp execution that is comprised of five
2D planes within a 3D domain. The resulting images created from this code is shown in
[multiapp-render-output].

!listing multiapps/multiapp.py
         start=import
         id=multiapp-render
         caption=Example code using the `MultiAppExodusReader` to visualize spatial MultiApp results.

!media chigger/multiapp.png
       id=multiapp-render-output
       style=width:30%;margin:auto;
       style=width:30%;margin:auto;
       caption=Output created by executing the script in [multiapp-render].


!chigger options object=chigger.exodus.MultiAppExodusReader

!chigger tests object=chigger.exodus.MultiAppExodusReader
