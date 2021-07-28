#include "HSBoundaryHeatFlux.h"
#include "HeatConductionModel.h"
#include "HeatStructureCylindricalBase.h"

registerMooseObject("THMApp", HSBoundaryHeatFlux);

InputParameters
HSBoundaryHeatFlux::validParams()
{
  InputParameters params = HSBoundary::validParams();

  params.addRequiredParam<FunctionName>("q", "Heat flux [W/m^2]");
  params.addParam<PostprocessorName>("scale_pp",
                                     "Post-processor by which to scale boundary condition");

  params.addClassDescription("Applies a specified heat flux to a heat structure boundary");

  return params;
}

HSBoundaryHeatFlux::HSBoundaryHeatFlux(const InputParameters & params)
  : HSBoundary(params),

    _q_fn_name(getParam<FunctionName>("q"))
{
}

void
HSBoundaryHeatFlux::check() const
{
  HSBoundary::check();

  if (isParamValid("scale_pp"))
  {
    const PostprocessorName & pp_name = getParam<PostprocessorName>("scale_pp");
    if (!_sim.hasPostprocessor(pp_name))
      logError("The post-processor name provided for the parameter 'scale_pp' is '" + pp_name +
               "', but no post-processor of this name exists.");
  }
}

void
HSBoundaryHeatFlux::addMooseObjects()
{
  const HeatStructureBase & hs = getComponent<HeatStructureBase>("hs");
  const HeatStructureCylindricalBase * hs_cyl =
      dynamic_cast<const HeatStructureCylindricalBase *>(&hs);
  const bool is_cylindrical = hs_cyl != nullptr;

  {
    const std::string class_name = is_cylindrical ? "ADHSHeatFluxRZBC" : "ADHSHeatFluxBC";
    InputParameters pars = _factory.getValidParams(class_name);
    pars.set<NonlinearVariableName>("variable") = HeatConductionModel::TEMPERATURE;
    pars.set<std::vector<BoundaryName>>("boundary") = _boundary;
    pars.set<FunctionName>("function") = _q_fn_name;
    if (is_cylindrical)
    {
      pars.set<Point>("axis_point") = hs.getPosition();
      pars.set<RealVectorValue>("axis_dir") = hs.getDirection();
      pars.set<Real>("offset") = hs_cyl->getInnerRadius() - hs_cyl->getAxialOffset();
    }
    if (isParamValid("scale_pp"))
      pars.set<PostprocessorName>("scale_pp") = getParam<PostprocessorName>("scale_pp");

    _sim.addBoundaryCondition(class_name, genName(name(), "bc"), pars);
  }

  // Create integral PP for cylindrical heat structures
  if (is_cylindrical)
  {
    const std::string class_name = "FunctionSideIntegralRZ";
    InputParameters pars = _factory.getValidParams(class_name);
    pars.set<std::vector<BoundaryName>>("boundary") = _boundary;
    pars.set<FunctionName>("function") = _q_fn_name;
    pars.set<Point>("axis_point") = hs.getPosition();
    pars.set<RealVectorValue>("axis_dir") = hs.getDirection();
    pars.set<Real>("offset") = hs_cyl->getInnerRadius() - hs_cyl->getAxialOffset();
    pars.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_TIMESTEP_END};
    _sim.addPostprocessor(class_name, genName(name(), "integral"), pars);
  }
}
