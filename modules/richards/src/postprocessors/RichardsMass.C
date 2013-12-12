//  This post processor returns the mass value of a region.  It is used
//  to check that mass is conserved
//
#include "RichardsMass.h"

template<>
InputParameters validParams<RichardsMass>()
{
  InputParameters params = validParams<ElementIntegralVariablePostprocessor>();
  params.addClassDescription("Returns the mass in a region.");
  return params;
}

RichardsMass::RichardsMass(const std::string & name, InputParameters parameters) :
    ElementIntegralVariablePostprocessor(name, parameters),

    _this_var_num(_var.index()),
    _p_var_nums(getMaterialProperty<std::vector<unsigned int> >("p_var_nums")),

    _porosity(getMaterialProperty<Real>("porosity")),
    _sat(getMaterialProperty<std::vector<Real> >("sat")),
    _density(getMaterialProperty<std::vector<Real> >("density"))
{
}

Real
RichardsMass::computeQpIntegral()
{
  for (int pvar=0 ; pvar<_p_var_nums.size() ; ++pvar )
    {
      if (_p_var_nums[_qp][pvar] == _this_var_num)
	{
	  return _porosity[_qp]*_density[_qp][pvar]*_sat[_qp][pvar];
	}
    }
  return 0.0;
}
