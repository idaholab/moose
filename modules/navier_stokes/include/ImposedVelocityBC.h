#ifndef IMPOSEDVELOCITYBC_H
#define IMPOSEDVELOCITYBC_H

// The base class definition (part of MOOSE)
#include "NodalBC.h"

//Forward Declarations
class ImposedVelocityBC;


// Specialization required of all user-level Moose objects
template<>
InputParameters validParams<ImposedVelocityBC>();

class ImposedVelocityBC : public NodalBC
{
public:
  // Constructor
  ImposedVelocityBC(const std::string & name, InputParameters parameters);

  // Destructor, better be virtual
  virtual ~ImposedVelocityBC(){}

protected:

  // Your BC should at least specialize the computeQpResidual function
  virtual Real computeQpResidual();

  // We need the density, since we are actually setting essential values of
  // *momentum* not essential values of velocity.
  VariableValue & _rho;

  // The desired value for the velocity component
  Real _desired_velocity;
};


#endif // IMPOSEDVELOCITYBC_H
