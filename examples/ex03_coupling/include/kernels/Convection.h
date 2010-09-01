#ifndef CONVECTION_H
#define CONVECTION_H

#include "Kernel.h"

//Forward Declarations
class Convection;

template<>
InputParameters validParams<Convection>();

class Convection : public Kernel
{
public:

  Convection(std::string name,
             MooseSystem &sys,
             InputParameters parameters);

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

private:
  /**
   * Coupled things come through as VaribleXYZ _references_.
   *
   * Since this is a reference it MUST be set in the Initialization List of the
   * constructor!
   */
  VariableGradient & _velocity_vector;
};

#endif //CONVECTION_H
