/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

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
