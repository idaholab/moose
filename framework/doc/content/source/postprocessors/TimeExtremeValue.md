# TimeExtremeValue

The TimeExtremeValue Postprocessor reports the extreme value of coupled Postprocessor seen over time.
For example, this Postprocessor can be used to record the peak temperature or maximum stress in a
simulation with an oscillating force. The minimum, absolute minimum, and absolute maximum over time
are also options in addition to the default maximum.

The parameter [!param](/Postprocessors/TimeExtremeValue/output_type) allows the user to select whether to output the extreme value requested as described above or the time at which the extreme value occurred.

!syntax description /Postprocessors/TimeExtremeValue

!syntax parameters /Postprocessors/TimeExtremeValue

!syntax inputs /Postprocessors/TimeExtremeValue

!syntax children /Postprocessors/TimeExtremeValue

!bibtex bibliography
