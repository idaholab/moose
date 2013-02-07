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

#ifndef SETUPQUADRATUREACTION_H_
#define SETUPQUADRATUREACTION_H_

#include "Action.h"
// libMesh
#include "libmesh/enum_order.h"
#include "libmesh/enum_quadrature_type.h"

class SetupQuadratureAction;

template<>
InputParameters validParams<SetupQuadratureAction>();

/**
 * Sets the quadrature
 */
class SetupQuadratureAction : public Action
{
public:
  SetupQuadratureAction(const std::string & name, InputParameters parameters);
  virtual ~SetupQuadratureAction();

protected:
  virtual void act();

  QuadratureType _type;
  Order _order;
};


#endif /* SETUPQUADRATUREACTION_H_ */
