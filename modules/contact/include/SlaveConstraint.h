#ifndef SLAVECONSTRAINT_H
#define SLAVECONSTRAINT_H

#include "ContactMaster.h" // For the ContactModel

// Moose Includes
#include "DiracKernel.h"
#include "PenetrationLocator.h"

//Forward Declarations
class SlaveConstraint;

template<>
InputParameters validParams<SlaveConstraint>();

class SlaveConstraint : public DiracKernel
{
public:
  SlaveConstraint(const std::string & name, InputParameters parameters);

  virtual void addPoints();
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

protected:
  const unsigned int _component;
  const ContactModel _model;
  const ContactFormulation _formulation;
  PenetrationLocator & _penetration_locator;

  const Real _penalty;
  const Real _friction_coefficient;

  NumericVector<Number> & _residual_copy;

  std::map<Point, PenetrationInfo *> _point_to_info;

  const unsigned int _x_var;
  const unsigned int _y_var;
  const unsigned int _z_var;

  const unsigned int _mesh_dimension;

  const RealVectorValue _vars;
};

#endif //SLAVECONSTRAINT_H
