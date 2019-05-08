#include "HeatStructureCylindrical.h"
#include "SolidMaterialProperties.h"
#include "ConstantFunction.h"
#include "Enums.h"
#include "THMMesh.h"

registerMooseObject("THMApp", HeatStructureCylindrical);

template <>
InputParameters
validParams<HeatStructureCylindrical>()
{
  InputParameters params = validParams<HeatStructureBase>();
  params.addParam<Real>("axial_offset", 0., "Axial offset for the undisplaced mesh");
  params.addClassDescription("Cylindrical heat structure");
  return params;
}

HeatStructureCylindrical::HeatStructureCylindrical(const InputParameters & params)
  : HeatStructureBase(params), _axial_offset(getParam<Real>("axial_offset"))
{
  if (_width.size() == _number_of_hs)
  {
    std::vector<Real> r(_number_of_hs + 1, _axial_offset);
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
HeatStructureCylindrical::getUnitPerimeter(const MooseEnum & side) const
{
  if (side == "top")
    return 2 * M_PI * (_axial_offset + _total_width);
  else if (side == "bottom")
    return 2 * M_PI * _axial_offset;
  else
    mooseError(name(), ": The heat structure side value '", side, "' is invalid.");
}
