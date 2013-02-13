/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef MOOSEEXCEPTION_H
#define MOOSEEXCEPTION_H

/**
 * MooseException is typedef'd as int so we can do simple sync'ing over the MPI.
 *
 * To use it in MOOSE (framework, not applications!), you have two macros available:
 * - PARALLEL_TRY
 * - PARALLEL_CATCH
 *
 * To use these right, one has to include a critical part of the code inside it. There cannot
 * be any MPI communications inside this block, since it would mess up the MPI comm pattern and
 * the code would end up out of sync (i.e. hang inside MPI).
 *
 * NOTE: These macros can be used only inside Nonlinear system (right now).
 */
typedef int MooseException;

#endif /* MOOSEEXCEPTION_H */
