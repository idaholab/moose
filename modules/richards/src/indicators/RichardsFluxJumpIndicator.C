/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

#include "RichardsFluxJumpIndicator.h"

template<>
InputParameters validParams<RichardsFluxJumpIndicator>()
{
  InputParameters params = validParams<JumpIndicator>();
  params.addRequiredParam<UserObjectName>("porepressureNames_UO", "The UserObject that holds the list of porepressure names.");
  params.addClassDescription("Indicator which calculates jumps in Richards fluxes over elements.");
  return params;
}


RichardsFluxJumpIndicator::RichardsFluxJumpIndicator(const std::string & name, InputParameters parameters) :
    JumpIndicator(name, parameters),

    _pp_name_UO(getUserObject<RichardsPorepressureNames>("porepressureNames_UO")),
    _pvar(_pp_name_UO.pressure_var_num(_var.index())),

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
  RealVectorValue gra = _density[_qp][_pvar]*_rel_perm[_qp][_pvar]*(_permeability[_qp]*(_grad_u[_qp] - _density[_qp][_pvar]*_gravity[_qp]));
  RealVectorValue gra_n = _density_n[_qp][_pvar]*_rel_perm_n[_qp][_pvar]*(_permeability_n[_qp]*(_grad_u_neighbor[_qp] - _density_n[_qp][_pvar]*_gravity_n[_qp]));

  Real jump = (gra - gra_n)*_normals[_qp];

  return jump*jump;
}

