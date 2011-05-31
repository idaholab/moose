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
  //Real temperature(); // temperature is now an aux var
  virtual Real computeQpResidual();

  unsigned int _p_var;
  VariableValue & _p;

//  unsigned int _u_vel_var;
//  VariableValue & _u_vel;
//
//  unsigned int _v_vel_var;
//  VariableValue & _v_vel;
//
//  unsigned int _w_vel_var;
//  VariableValue & _w_vel;

  unsigned int _c_v_var;
  VariableValue & _c_v;

  Real _initial;
  Real _final;
  Real _duration;

  // MaterialProperty<Real> & _gamma; // Can we use mat prop in NodalBC?
  // MaterialProperty<Real> & _R; // Can we use mat prop in nodal BC?
  // MaterialProperty<Real> & _c_v;
};

#endif //THERMALBC_H
