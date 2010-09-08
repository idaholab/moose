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

#ifndef ELEMENTL2ERROR_H
#define ELEMENTL2ERROR_H

#include "ElementIntegral.h"

//Forward Declarations
class ElementL2Error;

template<>
InputParameters validParams<ElementL2Error>();

class ElementL2Error : public ElementIntegral
{
public:
  ElementL2Error(const std::string & name, MooseSystem & moose_system, InputParameters parameters);
  
  /**
   * Get the L2 Error.
   */
  virtual Real getValue();

protected:
  virtual Real computeQpIntegral();

private:
  Function & _func;
};

#endif //ELEMENTL2ERROR_H
