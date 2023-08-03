# HeatConductionModel

## Overview

The heat conduction model is a THM utility in charge of adding the heat conduction
equations to heat structure boundary components.

It creates:

- the temperature variable. It currently defaults to first-order Lagrange.
- an initial conduction for temperature based on the interface temperature.
- kernels for the heat conduction equations in either XYZ or RZ coordinates, with
  the time derivative if solving a transient.
