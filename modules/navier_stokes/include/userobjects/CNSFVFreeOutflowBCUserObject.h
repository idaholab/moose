/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef CNSFVFREEOUTFLOWBCUSEROBJECT_H
#define CNSFVFREEOUTFLOWBCUSEROBJECT_H

#include "BCUserObject.h"
#include "SinglePhaseFluidProperties.h"

class CNSFVFreeOutflowBCUserObject;

template <>
InputParameters validParams<CNSFVFreeOutflowBCUserObject>();

/**
 * A user object that computes the ghost cell values based on the free outflow boundary condition
 */
class CNSFVFreeOutflowBCUserObject : public BCUserObject
{
public:
  CNSFVFreeOutflowBCUserObject(const InputParameters & parameters);

  virtual std::vector<Real> getGhostCellValue(unsigned int iside,
                                              dof_id_type ielem,
                                              const std::vector<Real> & uvec1,
                                              const RealVectorValue & dwave) const;

protected:
  const SinglePhaseFluidProperties & _fp;
};

#endif
