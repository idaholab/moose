#include "IdealRealTwoPhaseNCGFluidProperties.h"
#include "VaporMixtureFluidProperties.h"

registerMooseObject("THMApp", IdealRealTwoPhaseNCGFluidProperties);

template <>
InputParameters
validParams<IdealRealTwoPhaseNCGFluidProperties>()
{
  InputParameters params = validParams<TwoPhaseNCGFluidProperties>();

  params.addClassDescription(
      "Fluid properties for 2-phase fluid with an arbitrary mixture of non-condensable gases");

  params.makeParamRequired<UserObjectName>("fp_2phase");
  params.addRequiredParam<std::vector<UserObjectName>>(
      "fp_ncgs", "Name of fluid properties user object(s) for non-condensable gases");

  return params;
}

IdealRealTwoPhaseNCGFluidProperties::IdealRealTwoPhaseNCGFluidProperties(
    const InputParameters & parameters)
  : TwoPhaseNCGFluidProperties(parameters)
{
  _fp_2phase = &_fe_problem.getUserObjectTempl<TwoPhaseFluidProperties>(_2phase_name, _tid);

  // create vapor mixture fluid properties
  if (_tid == 0)
  {
    const std::string class_name = "IdealRealGasMixtureFluidProperties";
    InputParameters params = _app.getFactory().getValidParams(class_name);
    params.set<UserObjectName>("fp_primary") = getVaporName();
    params.set<std::vector<UserObjectName>>("fp_secondary") =
        getParam<std::vector<UserObjectName>>("fp_ncgs");
    _fe_problem.addUserObject(class_name, _vapor_mixture_name, params);
  }
  _fp_vapor_mixture =
      &_fe_problem.getUserObjectTempl<VaporMixtureFluidProperties>(_vapor_mixture_name, _tid);
}
