#ifndef THERMALBC_H
#define THERMALBC_H

#include "NodalBC.h"
#include "Material.h"


//Forward Declarations
class ThermalBC;

template<>
InputParameters validParams<ThermalBC>();

class ThermalBC : public NodalBC
{
public:

  ThermalBC(const std::string & name, InputParameters parameters);
  
  virtual ~ThermalBC(){}

protected:
  // Computes the temperature based on ideal gas equation of state,
  // the total energy, and the velocity: T = e_i/c_v
  virtual Real computeQpResidual();

  unsigned int _rho_var;
  VariableValue & _rho;

  Real _initial;
  Real _final;
  Real _duration;

  // Specific heat at constant volume, treated as a single
  // constant value.
  Real _cv;
};

#endif //THERMALBC_H
