# SphereContactor

!syntax description /UserObjects/SphereContactor

## Description

Analytic spherical rigid contactor.  Provides the signed distance
$g_{\text{LS}}(x) = \|x - c\| - R$ and outward unit normal
$\hat{n}(x) = (x - c) / \|x - c\|$ to the rigid-body contact stack
described in the [theory page](modules/contact/rigid_contact/theory.md).

The sphere center and radius are user-supplied at construction; the
Cartesian translations inherited from [LevelSetContactor.md] can be
prescribed as `Function`s of time (rigid-body kinematics) or coupled to
a `Scalar` variable (the [load-control layer](modules/contact/rigid_contact/theory.md#load-control)).

## Parameters

!syntax parameters /UserObjects/SphereContactor
