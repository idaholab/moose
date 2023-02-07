//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HeatSourceFromTotalPower.h"
#include "HeatStructureCylindricalBase.h"
#include "TotalPowerBase.h"

registerMooseObject("ThermalHydraulicsApp", HeatSourceFromTotalPower);

InputParameters
HeatSourceFromTotalPower::validParams()
{
  InputParameters params = HeatSourceBase::validParams();
  params.addRequiredParam<std::string>("power", "Component that provides total power");
  params.addParam<Real>(
      "power_fraction", 1., "Fraction of the total power that goes into the heat structure [-]");
  params.addParam<FunctionName>("power_shape_function", "Axial power shape [-]");
  params.declareControllable("power_fraction");
  params.addClassDescription("Heat generation from total power");
  return params;
}

HeatSourceFromTotalPower::HeatSourceFromTotalPower(const InputParameters & parameters)
  : HeatSourceBase(parameters),
    _power_fraction(getParam<Real>("power_fraction")),
    _has_psf(isParamValid("power_shape_function")),
    _power_shape_func(_has_psf ? getParam<FunctionName>("power_shape_function") : "")
{
  checkSizeGreaterThan<std::string>("regions", 0);
}

void
HeatSourceFromTotalPower::init()
{
  HeatSourceBase::init();

  if (hasComponent<TotalPowerBase>("power"))
  {
    const TotalPowerBase & rp = getComponent<TotalPowerBase>("power");
    _power_var_name = rp.getPowerVariableName();
  }
}

void
HeatSourceFromTotalPower::check() const
{
  HeatSourceBase::check();

  checkComponentOfTypeExists<TotalPowerBase>("power");
}

void
HeatSourceFromTotalPower::addMooseObjects()
{
  /// The heat structure component we work with
  const HeatStructureInterface & hs = getComponent<HeatStructureInterface>("hs");
  const HeatStructureBase * hs_base = dynamic_cast<const HeatStructureBase *>(&hs);

  Real n_units, length, P_unit;
  if (hs_base)
  {
    n_units = hs_base->getNumberOfUnits();
    length = hs_base->getLength();
    P_unit = hs_base->getUnitPerimeter(HeatStructureSideType::OUTER);
  }
  else // HeatStructureFromFile3D
  {
    n_units = 1.0;
    length = 1.0;
    P_unit = 1.0;
  }

  const HeatStructureCylindricalBase * hs_cyl =
      dynamic_cast<const HeatStructureCylindricalBase *>(&hs);
  const bool is_cylindrical = hs_cyl != nullptr;

  if (!_has_psf)
  {
    _power_shape_func = genName(name(), "power_shape_fn");
    std::string class_name = "ConstantFunction";
    InputParameters pars = _factory.getValidParams(class_name);
    pars.set<Real>("value") = 1. / length;
    getTHMProblem().addFunction(class_name, _power_shape_func, pars);
  }

  const std::string power_shape_integral_name = _has_psf
                                                    ? genName(name(), _power_shape_func, "integral")
                                                    : genName(_power_shape_func, "integral");

  {
    const std::string class_name =
        is_cylindrical ? "FunctionElementIntegralRZ" : "FunctionElementIntegral";
    InputParameters pars = _factory.getValidParams(class_name);
    pars.set<std::vector<SubdomainName>>("block") = _subdomain_names;
    pars.set<FunctionName>("function") = _power_shape_func;
    if (is_cylindrical)
    {
      pars.set<Point>("axis_point") = hs_cyl->getPosition();
      pars.set<RealVectorValue>("axis_dir") = hs_cyl->getDirection();
      pars.set<Real>("offset") = hs_cyl->getInnerRadius() - hs_cyl->getAxialOffset();
    }
    pars.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL};
    // TODO: This seems to produce incorrect output files, even though this is the line
    // that should be here, becuase we don't want this PPS to be in any output. The effect
    // of this line is correct, but for some reason, MOOSE will start output scalar variables
    // even though we did not ask it to do so. Refs idaholab/thm#
    // pars.set<std::vector<OutputName>>("outputs") = getTHMProblem().getOutputsVector("none");
    getTHMProblem().addPostprocessor(class_name, power_shape_integral_name, pars);
  }

  {
    const std::string class_name =
        is_cylindrical ? "ADHeatStructureHeatSourceRZ" : "ADHeatStructureHeatSource";
    InputParameters pars = _factory.getValidParams(class_name);
    pars.set<NonlinearVariableName>("variable") = HeatConductionModel::TEMPERATURE;
    pars.set<std::vector<SubdomainName>>("block") = _subdomain_names;
    pars.set<Real>("num_units") = n_units;
    pars.set<Real>("power_fraction") = _power_fraction;
    pars.set<FunctionName>("power_shape_function") = _power_shape_func;
    pars.set<std::vector<VariableName>>("total_power") =
        std::vector<VariableName>(1, _power_var_name);
    if (is_cylindrical)
    {
      pars.set<Point>("axis_point") = hs_cyl->getPosition();
      pars.set<RealVectorValue>("axis_dir") = hs_cyl->getDirection();
      pars.set<Real>("offset") = hs_cyl->getInnerRadius() - hs_cyl->getAxialOffset();
    }
    else
    {
      // For plate heat structure, the element integral of the power shape only
      // integrates over x and y, not z, so the depth still needs to be applied.
      // getUnitPerimeter() with an arbitrary side gives the depth.
      pars.set<Real>("scale") = 1.0 / P_unit;
    }
    pars.set<PostprocessorName>("power_shape_integral_pp") = power_shape_integral_name;
    std::string mon = genName(name(), "heat_src");
    getTHMProblem().addKernel(class_name, mon, pars);
    connectObject(pars, mon, "power_fraction");
  }
}
