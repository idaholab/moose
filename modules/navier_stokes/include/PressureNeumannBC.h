#ifndef PRESSURENEUMANNBC_H
#define PRESSURENEUMANNBC_H

#include "BoundaryCondition.h"
#include "Material.h"


//Forward Declarations
class PressureNeumannBC;

template<>
InputParameters validParams<PressureNeumannBC>();

class PressureNeumannBC : public BoundaryCondition
{
public:

  PressureNeumannBC(std::string name, InputParameters parameters, std::string var_name, unsigned int boundary_id, std::vector<std::string> coupled_to, std::vector<std::string> coupled_as);

  virtual ~PressureNeumannBC(){}

protected:

  /**
   * This is here because materials don't yet work on boundaries!
   */
  Real pressure();
  
  virtual Real computeQpResidual();
  

  std::vector<Real> & _p;

  std::vector<Real> & _pe;

  std::vector<Real> & _pu;
  std::vector<Real> & _pv;
  std::vector<Real> & _pw;

  int _component;
};

#endif //PRESSURENEUMANNBC_H
