#ifndef THERMALBC_H
#define THERMALBC_H

#include "BoundaryCondition.h"
#include "Material.h"


//Forward Declarations
class ThermalBC;

template<>
InputParameters validParams<ThermalBC>();

class ThermalBC : public BoundaryCondition
{
public:

  ThermalBC(const std::string & name, MooseSystem & moose_system, InputParameters parameters);
  
  virtual ~ThermalBC(){}

protected:

  Real temperature();
  virtual Real computeQpResidual();

  unsigned int _p_var;
  VariableValue & _p;

  unsigned int _u_vel_var;
  VariableValue & _u_vel;

  unsigned int _v_vel_var;
  VariableValue & _v_vel;

  unsigned int _w_vel_var;
  VariableValue & _w_vel;

  Real _initial;
  Real _final;
  Real _duration;

  MaterialProperty<Real> & _gamma;
  MaterialProperty<Real> & _R;
  MaterialProperty<Real> & _c_v;
};

#endif //THERMALBC_H
