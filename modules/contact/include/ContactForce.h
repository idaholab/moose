#ifndef CONTACTFORCE_H
#define CONTACTFORCE_H

#include "BoundaryCondition.h"

//libMesh includes
#include "vector_value.h"


//Forward Declarations
class ContactForce;

template<>
InputParameters validParams<ContactForce>();

/**
 * Implements a simple constant ContactForce BC where grad(u)=value on the boundary.
 * Uses the term produced from integrating the diffusion operator by parts.
 */
class ContactForce : public BoundaryCondition
{
public:

  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  ContactForce(const std::string & name, MooseSystem & moose_system, InputParameters parameters);
  
 virtual ~ContactForce(){}

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

private:
  VariableValue & _penetration;

  Real _penalty;

  unsigned int _component;

  MaterialProperty<Real> & _youngs_modulus;
};

#endif //NEUMANNBC_H
