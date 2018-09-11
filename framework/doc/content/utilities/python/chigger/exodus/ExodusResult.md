# ExodusResult

The `ExodusResult` object is the main means that data from a reader (e.g., [`ExodusReader`](/ExodusReader.md))

## Example

The following demonstrates the use of `ExodusResult` with a custom camera. The camera the settings
are available by pressing the `c` key with an interactive window.

!listing exodus/blocks/blocks.py
         start=import
         id=exodus-example
         caption=Example script using an `ExodusResult` with custom camera settings.

!media chigger/blocks.png
       style=width:30%;margin:auto;
       id=exodus-example-output
       caption=Example output using the `ExodusResult` with custom camera settings.

!chigger keybindings object=chigger.exodus.ExodusResult

!chigger options object=chigger.exodus.ExodusResult

!chigger tests object=chigger.exodus.ExodusResult
