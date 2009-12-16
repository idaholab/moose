#include "SolidMechanicsMaterial.h"

template<>
InputParameters validParams<SolidMechanicsMaterial>()
{
  InputParameters params;
  return params;
}

SolidMechanicsMaterial::SolidMechanicsMaterial(std::string name,
                                               InputParameters parameters,
                                               unsigned int block_id,
                                               std::vector<std::string> coupled_to,
                                               std::vector<std::string> coupled_as)
  :Material(name,parameters,block_id,coupled_to,coupled_as),
   _grad_disp_x(coupledGrad("disp_x")),
   _grad_disp_y(coupledGrad("disp_y")),
   _grad_disp_z(_dim == 3 ? coupledGrad("disp_z") : _grad_zero),
   _stress(declareTensorProperty("stress"))
{}
