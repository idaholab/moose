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

#ifndef XFEMEQUALVALUECONSTRAINT_H
#define XFEMEQUALVALUECONSTRAINT_H

// MOOSE includes
#include "ElemElemConstraint.h"
#include "MooseMesh.h"

// Forward Declarations
class XFEMEqualValueConstraint;

template<>
InputParameters validParams<XFEMEqualValueConstraint>();

class XFEMEqualValueConstraint : public ElemElemConstraint 
{
public:
  XFEMEqualValueConstraint(const InputParameters & parameters);
  virtual ~XFEMEqualValueConstraint();

protected:
  virtual void setqRuleNormal(ElementPairInfo & element_pair_info);

  virtual Real computeQpResidual(Moose::DGResidualType type);
  
  virtual Real computeQpJacobian(Moose::DGJacobianType type);

  Real _alpha;
  Real _jump;
  Real _jump_flux;
};

#endif /* XFEMEQUALVALUECONSTRAINT_H_ */
