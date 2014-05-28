/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

#include "RichardsFluxJumpIndicator.h"

template<>
InputParameters validParams<RichardsFluxJumpIndicator>()
{
  InputParameters params = validParams<JumpIndicator>();
  params.addRequiredParam<UserObjectName>("richardsVarNames_UO", "The UserObject that holds the list of Richards variable names.");
  params.addClassDescription("Indicator which calculates jumps in Richards fluxes over elements.");
  return params;
}


RichardsFluxJumpIndicator::RichardsFluxJumpIndicator(const std::string & name, InputParameters parameters) :
    JumpIndicator(name, parameters),

    _richards_name_UO(getUserObject<RichardsVarNames>("richardsVarNames_UO")),
    _pvar(_richards_name_UO.richards_var_num(_var.number())),

    _flux(getMaterialProperty<std::vector<RealVectorValue> >("flux")),

    _flux_n(getNeighborMaterialProperty<std::vector<RealVectorValue> >("flux"))
{
}


Real
RichardsFluxJumpIndicator::computeQpIntegral()
{
  Real jump = (_flux[_qp][_pvar] - _flux_n[_qp][_pvar])*_normals[_qp];
  return jump*jump;
}

