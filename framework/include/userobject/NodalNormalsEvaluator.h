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

#ifndef NODALNORMALSEVALUATOR_H
#define NODALNORMALSEVALUATOR_H

#include "NodalUserObject.h"


class NodalNormalsEvaluator;

template<>
InputParameters validParams<NodalNormalsEvaluator>();

/**
 * Works on top of NodalNormalsPreprocessor
 */
class NodalNormalsEvaluator : public NodalUserObject
{
public:
  NodalNormalsEvaluator(const std::string & name, InputParameters parameters);
  virtual ~NodalNormalsEvaluator();

  virtual void initialize();
  virtual void destroy();
  virtual void finalize();
  virtual void execute();
  virtual void threadJoin(const UserObject & uo);

protected:
  NumericVector<Number> & _nx;
  NumericVector<Number> & _ny;
  NumericVector<Number> & _nz;
};


#endif /* NODALNORMALSEVALUATOR_H */
