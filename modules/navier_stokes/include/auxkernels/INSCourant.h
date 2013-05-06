#ifndef INSCOURANT_H
#define INSCOURANT_H

#include "AuxKernel.h"

//Forward Declarations
class INSCourant;

template<>
InputParameters validParams<INSCourant>();

/**
 * Computes h_min / |u|
 */
class INSCourant : public AuxKernel
{
public:
  INSCourant(const std::string & name, InputParameters parameters);

  virtual ~INSCourant() {}

protected:
  virtual Real computeValue();

  // Velocity
  VariableValue& _u_vel;
  VariableValue& _v_vel;
  VariableValue& _w_vel;
};

#endif //VELOCITYAUX_H
