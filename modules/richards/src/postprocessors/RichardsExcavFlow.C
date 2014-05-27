/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

#include "RichardsExcavFlow.h"
#include "Function.h"
#include "Material.h"

template<>
InputParameters validParams<RichardsExcavFlow>()
{
  InputParameters params = validParams<SideIntegralVariablePostprocessor>();
  params.addRequiredParam<FunctionName>("excav_geom_function", "The function describing the excavation geometry (type RichardsExcavGeom)");
  params.addRequiredParam<UserObjectName>("richardsVarNames_UO", "The UserObject that holds the list of Richards variable names.");
  params.addClassDescription("Records total flow INTO an excavation (if quantity is positive then flow has occured from rock into excavation void)");
  return params;
}

RichardsExcavFlow::RichardsExcavFlow(const std::string & name, InputParameters parameters) :
    SideIntegralVariablePostprocessor(name, parameters),
    FunctionInterface(parameters),

    _richards_name_UO(getUserObject<RichardsVarNames>("richardsVarNames_UO")),
    _pvar(_richards_name_UO.richards_var_num(_var.number())),

    _viscosity(getMaterialProperty<std::vector<Real> >("viscosity")),
    _gravity(getMaterialProperty<RealVectorValue>("gravity")),
    _permeability(getMaterialProperty<RealTensorValue>("permeability")),
    _rel_perm(getMaterialProperty<std::vector<Real> >("rel_perm")),
    _density(getMaterialProperty<std::vector<Real> >("density")),
    _func(getFunction("excav_geom_function"))
{}

Real
RichardsExcavFlow::computeQpIntegral()
{
  return -_func.value(_t, _q_point[_qp])*_normals[_qp]*((_density[_qp][_pvar]*_rel_perm[_qp][_pvar]/_viscosity[_qp][_pvar])*(_permeability[_qp]*(_grad_u[_qp] - _density[_qp][_pvar]*_gravity[_qp])))*_dt ;
}
