#include "HeatStructurePlate.h"

registerMooseObject("THMApp", HeatStructurePlate);

InputParameters
HeatStructurePlate::validParams()
{
  InputParameters params = HeatStructureBase::validParams();
  params.addRequiredParam<Real>("depth", "Dimension of plate fuel in the third direction [m]");
  params.addClassDescription("Plate heat structure");
  return params;
}

HeatStructurePlate::HeatStructurePlate(const InputParameters & params)
  : HeatStructureBase(params), _depth(getParam<Real>("depth"))
{
  if (_width.size() == _number_of_hs)
  {
    for (unsigned int i = 0; i < _number_of_hs; i++)
      _volume.push_back(_num_rods * _width[i] * _depth * _length);
  }
}

void
HeatStructurePlate::addMooseObjects()
{
  HeatStructureBase::addMooseObjects();

  _hc_model->addHeatEquationXYZ();
}

Real
HeatStructurePlate::getUnitPerimeter(const HeatStructureSideType & side) const
{
  switch (side)
  {
    case HeatStructureSideType::OUTER:
    case HeatStructureSideType::INNER:
      return _depth;

    case HeatStructureSideType::START:
    case HeatStructureSideType::END:
      return std::numeric_limits<Real>::quiet_NaN();
  }

  mooseError(name(), ": Unknown value of 'side' parameter.");
}
