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

#ifndef BOUNDARYVALUEPPS_H
#define BOUNDARYVALUEPPS_H

#include "GeneralPostprocessor.h"

class BoundaryValuePPS;
class BoundaryUserObject;

template<>
InputParameters validParams<BoundaryValuePPS>();

/**
 * This PPS just retrieves the value from BoundaryUserObject
 */
class BoundaryValuePPS : public GeneralPostprocessor
{
public:
  BoundaryValuePPS(const std::string & name, InputParameters parameters);
  virtual ~BoundaryValuePPS();

  virtual void initialize();
  virtual void execute();
  virtual Real getValue();

protected:
  const BoundaryUserObject & _uo;
  Real _value;
};

#endif /* BOUNDARYVALUEPPS_H */
