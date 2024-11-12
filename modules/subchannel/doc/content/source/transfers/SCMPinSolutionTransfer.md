# SCMPinSolutionTransfer

!syntax description /Transfers/SCMPinSolutionTransfer

## Overview

<!-- -->

This class is used to transfer the pin associated variables: pin surface temperature `Tpin`, axial heat rate qprime `q_prime` and pin diameter `Dpin`
to the visualization mesh.

## Example Input File Syntax

!listing /examples/coupling/1pinSquare_thermomech_SCM/one_pin_problem.i block=pin_transfer language=cpp

!syntax parameters /Transfers/SCMPinSolutionTransfer

!syntax inputs /Transfers/SCMPinSolutionTransfer

!syntax children /Transfers/SCMPinSolutionTransfer
