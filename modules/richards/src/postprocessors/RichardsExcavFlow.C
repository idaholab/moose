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

    _this_var_num(_var.index()),
    _p_var_nums(getMaterialProperty<std::vector<unsigned int> >("p_var_nums")),

    _viscosity(getMaterialProperty<std::vector<Real> >("viscosity")),
    _gravity(getMaterialProperty<RealVectorValue>("gravity")),
    _permeability(getMaterialProperty<RealTensorValue>("permeability")),
    _rel_perm(getMaterialProperty<std::vector<Real> >("rel_perm")),
    _density(getMaterialProperty<std::vector<Real> >("density")), 
    _func(getFunction("excav_geom_function")),
    _feproblem(dynamic_cast<FEProblem &>(_subproblem))
{}

Real
RichardsExcavFlow::computeQpIntegral()
{
  for (int pvar=0 ; pvar<_p_var_nums.size() ; ++pvar )
    {
      if (_p_var_nums[_qp][pvar] == _this_var_num)
	{
	  return -_func.value(_t, _q_point[_qp])*_normals[_qp]*((_density[_qp][pvar]*_rel_perm[_qp][pvar]/_viscosity[_qp][pvar])*(_permeability[_qp]*(_grad_u[_qp] - _density[_qp][pvar]*_gravity[_qp]))); //*_feproblem.dt();
	}
    }
  return 0.0;
}
