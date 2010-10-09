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

#ifndef COMPUTEPOSTPROCESSORS_H
#define COMPUTEPOSTPROCESSORS_H

// forward declares
class NonlinearImplicitSystem;
template<class T> class NumericVector;

namespace Moose
{
  void compute_postprocessors (const NumericVector<Number>& soln, NonlinearImplicitSystem& sys);
}

#endif //COMPUTEPOSTPROCESSORS_H
