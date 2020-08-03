#include "HeatStructureCylindrical.h"
#include "SolidMaterialProperties.h"
#include "ConstantFunction.h"
#include "Enums.h"
#include "THMMesh.h"

registerMooseObject("THMApp", HeatStructureCylindrical);

InputParameters
HeatStructureCylindrical::validParams()
{
  InputParameters params = HeatStructureBase::validParams();
  params.addParam<Real>("inner_radius", 0., "Inner radius of the heat structure [m]");
  params.addClassDescription("Cylindrical heat structure");
  return params;
}

HeatStructureCylindrical::HeatStructureCylindrical(const InputParameters & params)
  : HeatStructureBase(params), _inner_radius(getParam<Real>("inner_radius"))
{
  if (_width.size() == _number_of_hs)
  {
    std::vector<Real> r(_number_of_hs + 1, _inner_radius);
    for (unsigned int i = 0; i < _number_of_hs; i++)
    {
      r[i + 1] = r[i] + _width[i];
      _volume.push_back(_num_rods * M_PI * (r[i + 1] * r[i + 1] - r[i] * r[i]) * _length);
    }
  }
}

void
HeatStructureCylindrical::addMooseObjects()
{
  HeatStructureBase::addMooseObjects();

  _hc_model->addHeatEquationRZ();
}

Real
HeatStructureCylindrical::getUnitPerimeter(const HeatStructureBase::SideType & side) const
{
  switch (side)
  {
    case HeatStructureBase::OUTER:
      return 2 * M_PI * (_inner_radius + _total_width);

    case HeatStructureBase::INNER:
      return 2 * M_PI * _inner_radius;

    case HeatStructureBase::START:
    case HeatStructureBase::END:
      return std::numeric_limits<Real>::quiet_NaN();
  }
}
