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

#ifndef SETUPQUADRATUREACTION_H
#define SETUPQUADRATUREACTION_H

#include "Action.h"
// libMesh
#include "libmesh/enum_order.h"
#include "libmesh/enum_quadrature_type.h"

class SetupQuadratureAction;

template <>
InputParameters validParams<SetupQuadratureAction>();

/**
 * Sets the quadrature
 */
class SetupQuadratureAction : public Action
{
public:
  SetupQuadratureAction(InputParameters parameters);

  virtual void act() override;

protected:
  QuadratureType _type;
  Order _order;
  Order _element_order;
  Order _side_order;
};

#endif /* SETUPQUADRATUREACTION_H */
