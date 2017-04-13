/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef CNSFVRIEMANNINVARIANTBCUSEROBJECT_H
#define CNSFVRIEMANNINVARIANTBCUSEROBJECT_H

#include "BCUserObject.h"
#include "SinglePhaseFluidProperties.h"

class CNSFVRiemannInvariantBCUserObject;

template <>
InputParameters validParams<CNSFVRiemannInvariantBCUserObject>();

/**
 * A user object that computes the ghost cell values based on the Riemann invariant boundary
 * condition
 */
class CNSFVRiemannInvariantBCUserObject : public BCUserObject
{
public:
  CNSFVRiemannInvariantBCUserObject(const InputParameters & parameters);

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
