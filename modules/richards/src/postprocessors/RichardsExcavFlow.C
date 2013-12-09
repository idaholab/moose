#include "RichardsExcavFlow.h"
#include "Function.h"
#include "Material.h"

template<>
InputParameters validParams<RichardsExcavFlow>()
{
  InputParameters params = validParams<SideIntegralVariablePostprocessor>();
  params.addRequiredParam<FunctionName>("excav_geom_function", "The function describing the excavation geometry (type RichardsExcavGeom)");
  params.addClassDescription("Records total flow INTO an excavation (if quantity is positive then flow has occured from rock into excavation voic)");
  return params;
}

RichardsExcavFlow::RichardsExcavFlow(const std::string & name, InputParameters parameters) :
    SideIntegralVariablePostprocessor(name, parameters),
    FunctionInterface(parameters),
    _dens0(getMaterialProperty<Real>("dens0")),
    _viscosity(getMaterialProperty<Real>("viscosity")),
    _gravity(getMaterialProperty<RealVectorValue>("gravity")),
    _permeability(getMaterialProperty<RealTensorValue>("permeability")),
    _rel_perm(getMaterialProperty<Real>("rel_perm")),
    _density(getMaterialProperty<Real>("density")), 
    _func(getFunction("excav_geom_function")),
    _feproblem(dynamic_cast<FEProblem &>(_subproblem))
{}

Real
RichardsExcavFlow::computeQpIntegral()
{
  return -_func.value(_t, _q_point[_qp])*_normals[_qp]*((_density[_qp]*_rel_perm[_qp]/_viscosity[_qp])*(_permeability[_qp]*(_grad_u[_qp] - _dens0[_qp]*_gravity[_qp]))); //*_feproblem.dt();
}
