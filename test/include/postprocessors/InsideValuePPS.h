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

#ifndef INSIDEVALUEPPS_H
#define INSIDEVALUEPPS_H

#include "GeneralPostprocessor.h"

class InsideValuePPS;
class InsideUserObject;

template<>
InputParameters validParams<InsideValuePPS>();

/**
 * This PPS just retrieves the value from InsideUserObject
 */
class InsideValuePPS : public GeneralPostprocessor
{
public:
  InsideValuePPS(const std::string & name, InputParameters parameters);
  virtual ~InsideValuePPS();

  virtual void initialize();
  virtual void execute();
  virtual Real getValue();

protected:
  const InsideUserObject & _uo;
  Real _value;
};

#endif /* INSIDEVALUEPPS_H */
