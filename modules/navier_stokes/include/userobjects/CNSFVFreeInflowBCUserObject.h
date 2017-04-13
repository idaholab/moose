/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef CNSFVFREEINFLOWBCUSEROBJECT_H
#define CNSFVFREEINFLOWBCUSEROBJECT_H

#include "BCUserObject.h"
#include "SinglePhaseFluidProperties.h"

class CNSFVFreeInflowBCUserObject;

template <>
InputParameters validParams<CNSFVFreeInflowBCUserObject>();

/**
 * A user object that computes the ghost cell values based on the free inflow boundary condition
 */
class CNSFVFreeInflowBCUserObject : public BCUserObject
{
public:
  CNSFVFreeInflowBCUserObject(const InputParameters & parameters);

  virtual std::vector<Real> getGhostCellValue(unsigned int iside,
                                              dof_id_type ielem,
                                              const std::vector<Real> & uvec1,
                                              const RealVectorValue & dwave) const;

protected:
  const SinglePhaseFluidProperties & _fp;

  Real _inf_rho;
  Real _inf_uadv;
  Real _inf_vadv;
  Real _inf_wadv;
  Real _inf_pres;
};

#endif
