# MOOSE Overview

--

## MOOSE Overview Contents

!subtoc

--

## MOOSE Team

!css width=49% float=left
* Derek Gaston
    * [derek.gaston@inl.gov](mailto:derek.gaston@inl.gov)
    * [@friedmud](https://twitter.com/friedmud)
* Cody Permann
    * [cody.permann@inl.gov](mailto:cody.permann@inl.gov)
    * [@permcody](https://twitter.com/permcody)
* David Andr&#353;
    * [david.andrs@inl.gov](mailto:david.andrs@inl.gov)
    * [@andrsdave](https://twitter.com/andrsdave)
* John Peterson
    * [jw.peterson@inl.gov](mailto:jw.peterson@inl.gov)
    * [@peterson512](https://twitter.com/peterson512)

!css width=49% float=right
* Jason Miller
    * [jason.miller@inl.gov](mailto:jason.miller@inl.gov)
    * [@mjmiller96](https://twitter.com/mjmiller96)
* Andrew Slaughter
    * [andrew.slaughter@inl.gov](mailto:andrew.slaughter@inl.gov)
    * [@aeslaughter98](https://twitter.com/aeslaughter98)
* Brian Alger
    * [brian.alger@inl.gov](mailto:brian.alger@inl.gov)
* Fande Kong
    * [fande.kong@inl.gov](mailto:fande.kong@inl.gov)
* Robert Carlsen
    * [Robert Carlsen](mailto:robert.carlsen@inl.gov)

--

## Tweet It!

 - \#mooseframework
 - Tweets with that hashtag show up on the [mooseframework.org](http://www.mooseframework.org) website
 - Post pictures or videos of your latest results!

--

## MOOSE: Multiphysics Object Oriented Simulation Environment

!image docs/media/bison_pellet_stack.png float=right width=19%

!css class="moose-two-column" float=left width=80%
* A framework for solving computational engineering problems in a well-planned, managed, and coordinated way
* **Designed to significantly reduce the expense and time required to develop new applications**
    * *Maximize Science/$*
    * *Designed to be easily extended and maintained*
    * *Efficient on both a few and many processors*
    * *Provides an object-oriented, pluggable system for defining all aspects of a simulation tool.*

--

!image docs/media/moose_full_core.png

--

## Capabilities

* 1D, 2D and 3D
    * User code agnostic of dimension
* Finite Element Based
    * Continuous and Discontinuous Galerkin (and Petrov Galerkin)
* Fully Coupled, Fully Implicit
* Unstructured Mesh
    * All shapes (Quads, Tris, Hexes, Tets, Pyramids, Wedges, ...)
    * Higher order geometry (curvilinear, etc.)
    * Reads and writes multiple formats
* Mesh Adaptivity
* Parallel
    * User code agnostic of parallelism
* High Order
    * User code agnostic of shape functions
    * p-Adaptivity
* Built-in Postprocessing
* And much more ...

--

## Code Platform

* Uses well-established libraries
* Implements robust and state-of-the-art solution methods

!image docs/media/moose_arch.png width=80%

--

## Rapid Development

!css font-size=large
| **Application** | **Physics** | **Results** | **Lines** |
| :- | :- | :-: | :-: |
| BISON | Thermo-mechanics, Chemical, diffusion, coupled mesoscale | 4 mo. | 3,000 |
| PRONGHORN | Neutronics, Porous flow, eigenvalue MARMOT 4th order phasefield mesoscale | 3 mo. | 7,000 |
| MARMOT | 4-th order phase-field meso-scale | 1 mo. | 6,000 |
| RAT | Porous ReActive Transport | 1 mo. | 1,500 |
| FALCON | Geo-mechanics, coupled mesoscale | 3 mo. | 3,000
| MAMBA | Chemical reactions, prescipitation, and porous flow | 5 wks. | 2,500 |
| HYRAX | Phase-field, ZrH precipitation | 3 mo. | 1,000 |
| PIKA | Multi-scale heat and mass transfer with phase-change | 3 mo. | 2,900 |

--

## MOOSE Application Architecture

!image docs/media/moose_blocks.png

--

## MOOSE Code Example

!image docs/media/code_example.png

--

## MOOSE Software Quality Practices

* MOOSE follows an NQA-1 (Nuclear Quality Assurance Level 1) development process.
* All commits to MOOSE undergo review using GitHub Pull Requests and must pass a set of application regression tests before they are available to our users.
* All changes must be accompanied by issue numbers and assessed an appropriate risk level.
* We maintain a regression test code coverage level of 80% or better at all times.
* We follow strict code style and formatting guidelines [(wiki/CodeStandards)](http://www.mooseframework.com/wiki/CodeStandards/).
* We process code comments with third-party tools to generate documentation.

--

!image docs/media/revised_moose_devel.png
