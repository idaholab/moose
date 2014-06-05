/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

#include "RichardsMobilityJumpIndicator.h"

template<>
InputParameters validParams<RichardsMobilityJumpIndicator>()
{
  InputParameters params = validParams<JumpIndicator>();
  params.addRequiredParam<UserObjectName>("richardsVarNames_UO", "The UserObject that holds the list of Richards variable names.");
  params.addParam<Real>("a", 0.1, "Jump = (mobility - mobility_neighbor)/(mobility + mobility_neighbor + a)^b");
  params.addParam<Real>("b", 2, "Jump = (mobility - mobility_neighbor)/(mobility + mobility_neighbor + a)^b");
  params.addClassDescription("Indicator which calculates jumps in Richards fluxes over elements.");
  return params;
}


RichardsMobilityJumpIndicator::RichardsMobilityJumpIndicator(const std::string & name, InputParameters parameters) :
    JumpIndicator(name, parameters),

    _richards_name_UO(getUserObject<RichardsVarNames>("richardsVarNames_UO")),
    _pvar(_richards_name_UO.richards_var_num(_var.number())),

    _a(getParam<Real>("a")),
    _b(getParam<Real>("b")),

    _rel_perm(getMaterialProperty<std::vector<Real> >("rel_perm")),

    _density(getMaterialProperty<std::vector<Real> >("density")),

    _viscosity(getMaterialProperty<std::vector<Real> >("viscosity")),

    _rel_perm_n(getNeighborMaterialProperty<std::vector<Real> >("rel_perm")),

    _density_n(getNeighborMaterialProperty<std::vector<Real> >("density")),

    _viscosity_n(getNeighborMaterialProperty<std::vector<Real> >("viscosity"))
{
}


Real
RichardsMobilityJumpIndicator::computeQpIntegral()
{
  Real mob = _rel_perm[_qp][_pvar]*_density[_qp][_pvar]/_viscosity[_qp][_pvar];
  Real mob_n = _rel_perm_n[_qp][_pvar]*_density_n[_qp][_pvar]/_viscosity_n[_qp][_pvar];
  Moose::out << "mob, mob_n = " << mob << " " << mob_n << "\n";
  Real jump = std::abs(mob - mob_n)/std::pow(mob + mob_n + _a, _b);
  return jump*jump;
}

