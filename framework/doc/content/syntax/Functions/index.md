# Functions System

## Overview

`Function`s are used to define functions depending only on spatial position and
time: $f(x,y,z,t)$. These objects can serve a wide variety of purposes, including
(but not limited to) the following:

- defining initial conditions,
- defining residual contributions (sources, boundary conditions, etc.), and
- defining post-processing quantities.

!syntax list /Functions objects=True actions=False subsystems=False

!syntax list /Functions objects=False actions=False subsystems=True

!syntax list /Functions objects=False actions=True subsystems=False
