# Introduction to Tensor Mechanics

This tutorial aims to guide new users through their first set of tensor
mechanics simulations. We'll start with a minimal setup and work our way up in
complexity to where this tutorial can hand off to the
[Introduction to Contact](contact/tutorials/introduction/index.md optional=True).

## [First steps](tensor_mechanics/tutorials/introduction/step01.md)

In this step we start off with a simple bare bones mechanics problem and think about
units in MOOSE.

## [Adding boundary conditions](tensor_mechanics/tutorials/introduction/step02.md)

Next we add boundary conditions and loading. We think about the validity of the
strain formulation and make a quick excursion into automatic differentiation.

## [Subdomains and subdomain-specific properties](tensor_mechanics/tutorials/introduction/step03.md)

In this step we'll be setting up two subdomains (regions of our sample) with
differing material properties.

### [Thermal expansion](tensor_mechanics/tutorials/introduction/step03a.md)

In this step we'll have a look at how thermal expansion can be modeled in MOOSE.

## [Multiple submeshes](tensor_mechanics/tutorials/introduction/step04.md)

We'll set up a pair of mesh separate unconnected mesh blocks,as a final
preparation before we start working with mechanical contact.

### [Volumetric locking](tensor_mechanics/tutorials/introduction/step04a.md)

We'll look at the effects of volumetric locking and how to mitigate them.
