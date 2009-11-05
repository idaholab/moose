#ifndef THERMALBC_H
#define THERMALBC_H

#include "BoundaryCondition.h"
#include "Material.h"


//Forward Declarations
class ThermalBC;

template<>
Parameters valid_params<ThermalBC>();

class ThermalBC : public BoundaryCondition
{
public:

  ThermalBC(std::string name, Parameters parameters, std::string var_name, unsigned int boundary_id, std::vector<std::string> coupled_to, std::vector<std::string> coupled_as);
  
  virtual ~ThermalBC(){}

protected:

  Real temperature();
  virtual Real computeQpResidual();

  unsigned int _p_var;
  std::vector<Real> & _p;

  unsigned int _u_vel_var;
  std::vector<Real> & _u_vel;

  unsigned int _v_vel_var;
  std::vector<Real> & _v_vel;

  unsigned int _w_vel_var;
  std::vector<Real> & _w_vel;

  Real _initial;
  Real _final;
  Real _duration;
};

#endif //THERMALBC_H
