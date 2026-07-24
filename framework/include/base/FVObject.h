//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

/**
 * A marker class denoting that an object belongs to the finite volume system of a solver system
 * that is solved with a nonlinear solve, e.g. an FVKernel, FVBoundaryCondition, or
 * FVInterfaceKernel.
 *
 * This cannot derive from MooseObject: the finite volume base classes already inherit MooseObject
 * through separate paths (FVKernel through ResidualObject, FVBoundaryCondition and
 * FVInterfaceKernel directly), so deriving here would introduce a diamond. It is instead a pure
 * marker that AttribFVObject cross-casts to, which lets a single warehouse query retrieve every
 * finite volume object regardless of which family it belongs to.
 *
 * Inheriting this is what makes a finite volume base class visible to those queries -- most
 * importantly the SetupInterface dispatch in NonlinearSystemBase. Any *new* finite volume base
 * class that does not derive from one of the three classes above must inherit this directly, or
 * its initialSetup()/timestepSetup()/customSetup() overrides will silently never be called.
 */
class FVObject
{
};
