//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef POROUSFLOWNEARESTQP_QP
#define POROUSFLOWNEARESTQP_QP

#include "PorousFlowMaterial.h"

// Forward Declarations
class PorousFlowNearestQp;

template <>
InputParameters validParams<PorousFlowNearestQp>();

/**
 * Material designed to provide the nearest quadpoint to each node
 * in the element
 */
class PorousFlowNearestQp : public PorousFlowMaterial
{
public:
  PorousFlowNearestQp(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  /// the nearest quadpoint
  MaterialProperty<unsigned int> & _nearest_qp;
};

#endif // POROUSFLOWNEARESTQP_H
