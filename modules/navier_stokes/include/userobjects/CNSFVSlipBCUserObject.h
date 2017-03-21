/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef CNSFVSLIPWALLBCUSEROBJECT_H
#define CNSFVSLIPWALLBCUSEROBJECT_H

#include "BCUserObject.h"
#include "SinglePhaseFluidProperties.h"

class CNSFVSlipBCUserObject;

template <>
InputParameters validParams<CNSFVSlipBCUserObject>();

/**
 * A user object that computes the ghost cell values based on the slip wall boundary condition
 */
class CNSFVSlipBCUserObject : public BCUserObject
{
public:
  CNSFVSlipBCUserObject(const InputParameters & parameters);

  virtual std::vector<Real> getGhostCellValue(unsigned int iside,
                                              dof_id_type ielem,
                                              const std::vector<Real> & uvec1,
                                              const RealVectorValue & dwave) const;

protected:
  const SinglePhaseFluidProperties & _fp;
};

#endif
