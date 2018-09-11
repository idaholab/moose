# NemesisReader

The distributed version of Exodus files is known as Nemesis, where there is an independent
file for each processor. For this case it is necessary to use the `NemesisReader` object.

## Example

The `NemesisReader` works in similar fashion as the other readers, as shown in the example
below. This example uses the "explode" feature that separates the portions on each
processor for visualization purposes (see [nemesis-example-output]).

!listing explode.py
         start=import
         id=nemesis-example
         caption=Example use of `NemesisReader`.

!media exploder.png
       style=width:30%;margin:auto;
       id=nemesis-example-output
       caption=Example output using the `NemesisReader` with the explode feature.

!chigger options object=chigger.exodus.NemesisReader

!chigger tests object=chigger.exodus.NemesisReader
