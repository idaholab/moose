# ExodusReader

The `ExodusReader` object is designed to read ExodusII files for use with [ChiggerResult](ChiggerResult.md)
objects. The reader alone simple provides an interface for accessing the various data within
an Exodus file including time information as well as nodal, elemental, and global variables. The
reader automatically handles adaptive files with the "-s" suffix.

## Example

As a stand alone object the reader may be useful for gathering information regarding a file. The
following code loads the file and then prints information regarding the file as shown in
[reader-output].

!listing exodus/reader.py start=import

!listing! id=reader-output
          language=text
          caption=Example output from printing `ExodusReader` object.
Timesteps: 21
    Times: [0.0, 0.1, 0.2, 0.30000000000000004, 0.4, 0.5, 0.6, 0.7, 0.7999999999999999, 0.8999999999999999, 0.9999999999999999, 1.0999999999999999, 1.2, 1.3, 1.4000000000000001, 1.5000000000000002, 1.6000000000000003, 1.7000000000000004, 1.8000000000000005, 1.9000000000000006, 2.0000000000000004]

Nodal Variables:
  convected
      components: 1
           range: (0.0, 1.0000000000000018)
  diffused
      components: 1
           range: (0.0, 2.0000000000000036)

Elemental Variables:
  aux_elem
      components: 1
           range: (0.009018163476295126, 9.991707387689374)

Postprocessors:
  func_pp
      components: 1

Subdomains:
  1
  76

Boundaries:
  bottom (1)
  top (2)

Nodesets:
  1
  2
!listing-end!

!chigger options object=chigger.exodus.ExodusReader

!chigger tests object=chigger.exodus.ExodusReader
