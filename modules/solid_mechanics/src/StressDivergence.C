#include "StressDivergence.h"

#include "Material.h"

template<>
InputParameters validParams<StressDivergence>()
{
  InputParameters params;
  params.addRequiredParam<Real>("component", "An integer corresponding to the direction the variable this kernel acts in. (0 for x, 1 for y, 2 for z)");
  return params;
}


StressDivergence::StressDivergence(std::string name,
            InputParameters parameters,
            std::string var_name,
            std::vector<std::string> coupled_to,
            std::vector<std::string> coupled_as)
  :Kernel(name,parameters,var_name,true,coupled_to,coupled_as),
   _component(parameters.get<Real>("component"))
{}

void
StressDivergence::subdomainSetup()
{
  _stress = &_material->getTensorProperty("stress");
}

Real
StressDivergence::computeQpResidual()
{
  Real r = (*_stress)[_qp].row(_component) * _dtest[_i][_qp];
  
  return r;
}

Real
StressDivergence::computeQpJacobian()
{
  return 0;
}
