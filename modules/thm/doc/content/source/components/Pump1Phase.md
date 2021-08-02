# Pump1Phase

!syntax description /Components/Pump1Phase

This component implements a pump model for 1-phase flow that has a volume,
which is directly supplied by the user. This component is heavily based off of
VolumeJunction1Phase. The main difference is that there is only one momentum
conservation equation (the ones for rhovV and rhowV were removed) since the pump
inlet and outlet are aligned in the rhouV direction. Also there are new pump
head terms added to the momentum and energy equations.

!syntax parameters /Components/Pump1Phase

!syntax inputs /Components/Pump1Phase

!syntax children /Components/Pump1Phase

!bibtex bibliography
