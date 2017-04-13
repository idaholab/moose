/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef SLAVECONSTRAINT_H
#define SLAVECONSTRAINT_H

#include "ContactMaster.h" // For the ContactModel

// Moose Includes
#include "DiracKernel.h"
#include "PenetrationLocator.h"

// Forward Declarations
class SlaveConstraint;

template <>
InputParameters validParams<SlaveConstraint>();

class SlaveConstraint : public DiracKernel
{
public:
  SlaveConstraint(const InputParameters & parameters);

  virtual void addPoints();
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

protected:
  Real nodalArea(PenetrationInfo & pinfo);

  const unsigned int _component;
  const ContactModel _model;
  const ContactFormulation _formulation;
  const bool _normalize_penalty;
  PenetrationLocator & _penetration_locator;

  const Real _penalty;
  const Real _friction_coefficient;

  NumericVector<Number> & _residual_copy;

  std::map<Point, PenetrationInfo *> _point_to_info;

  std::vector<unsigned int> _vars;

  const unsigned int _mesh_dimension;

  MooseVariable * _nodal_area_var;
  SystemBase & _aux_system;
  const NumericVector<Number> * _aux_solution;
};

#endif // SLAVECONSTRAINT_H
