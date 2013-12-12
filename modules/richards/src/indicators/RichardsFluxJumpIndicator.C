#include "RichardsFluxJumpIndicator.h"

template<>
InputParameters validParams<RichardsFluxJumpIndicator>()
{
  InputParameters params = validParams<JumpIndicator>();
  params.addClassDescription("Indicator which calculates jumps in Richards fluxes over elements.");
  return params;
}


RichardsFluxJumpIndicator::RichardsFluxJumpIndicator(const std::string & name, InputParameters parameters) :
    JumpIndicator(name, parameters),

    _this_var_num(_var.index()),
    _p_var_nums(getMaterialProperty<std::vector<unsigned int> >("p_var_nums")),

    _density(getMaterialProperty<std::vector<Real> >("density")),
    _rel_perm(getMaterialProperty<std::vector<Real> >("rel_perm")),
    _gravity(getMaterialProperty<RealVectorValue>("gravity")),
    _permeability(getMaterialProperty<RealTensorValue>("permeability")),

    _density_n(getNeighborMaterialProperty<std::vector<Real> >("density")),
    _rel_perm_n(getNeighborMaterialProperty<std::vector<Real> >("rel_perm")),
    _gravity_n(getNeighborMaterialProperty<RealVectorValue>("gravity")),
    _permeability_n(getNeighborMaterialProperty<RealTensorValue>("permeability"))
{
}


Real
RichardsFluxJumpIndicator::computeQpIntegral()
{
  for (int pvar=0 ; pvar<_p_var_nums.size() ; ++pvar )
    {
      if (_p_var_nums[_qp][pvar] == _this_var_num)
	{
	  RealVectorValue gra = _density[_qp][pvar]*_rel_perm[_qp][pvar]*(_permeability[_qp]*(_grad_u[_qp] - _density[_qp][pvar]*_gravity[_qp]));
	  RealVectorValue gra_n = _density_n[_qp][pvar]*_rel_perm_n[_qp][pvar]*(_permeability_n[_qp]*(_grad_u_neighbor[_qp] - _density_n[_qp][pvar]*_gravity_n[_qp]));
	  
	  Real jump = (gra - gra_n)*_normals[_qp];
	  
	  return jump*jump;
	}
    }
  return 0.0;
}

