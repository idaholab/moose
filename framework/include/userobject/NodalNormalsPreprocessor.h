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

#ifndef NODALNORMALSPREPROCESSOR_H
#define NODALNORMALSPREPROCESSOR_H

#include "ElementUserObject.h"


class NodalNormalsPreprocessor;

template<>
InputParameters validParams<NodalNormalsPreprocessor>();

/**
 *
 */
class NodalNormalsPreprocessor : public ElementUserObject
{
public:
  NodalNormalsPreprocessor(const std::string & name, InputParameters parameters);
  virtual ~NodalNormalsPreprocessor();

  virtual void initialize();
  virtual void destroy();
  virtual void finalize();
  virtual void execute();
  virtual void threadJoin(const UserObject & uo);

protected:
  NumericVector<Number> & _nx;
  NumericVector<Number> & _ny;
  NumericVector<Number> & _nz;

  const VariablePhiGradient & _grad_phi;
};


#endif /* NODALNORMALSPREPROCESSOR_H */
