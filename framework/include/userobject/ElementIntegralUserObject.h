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

#ifndef ELEMENTINTEGRALUSEROBJECT_H
#define ELEMENTINTEGRALUSEROBJECT_H

#include "ElementPostprocessor.h"

//Forward Declarations
class ElementIntegralUserObject;

template<>
InputParameters validParams<ElementIntegralUserObject>();

/**
 * This postprocessor computes a volume integral of the specified variable.
 *
 * Note that specializations of this integral are possible by deriving from this
 * class and overriding computeQpIntegral().
 */
class ElementIntegralUserObject : public ElementUserObject
{
public:
  ElementIntegralUserObject(const std::string & name, InputParameters parameters);

  virtual void initialize();
  virtual void execute();
  virtual void threadJoin(const UserObject & y);
  virtual Real getValue();

  virtual void finalize(){}

protected:
  virtual Real computeQpIntegral() = 0;
  virtual Real computeIntegral();

  unsigned int _qp;

  Real _integral_value;
};

#endif
