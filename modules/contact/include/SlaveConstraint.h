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

  virtual void jacobianSetup();
  virtual void timestepSetup();

  virtual void addPoints();
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  virtual void updateContactSet();

protected:
  const unsigned int _component;
  const ContactModel _model;
  PenetrationLocator & _penetration_locator;

  Real _penalty;

  NumericVector<Number> & _residual_copy;

  std::map<Point, PenetrationLocator::PenetrationInfo *> _point_to_info;

  unsigned int _x_var;
  unsigned int _y_var;
  unsigned int _z_var;

  RealVectorValue _vars;
};

#endif //SLAVECONSTRAINT_H
