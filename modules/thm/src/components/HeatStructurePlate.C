#include "HeatStructurePlate.h"

registerMooseObject("THMApp", HeatStructurePlate);

template <>
InputParameters
validParams<HeatStructurePlate>()
{
  InputParameters params = validParams<HeatStructureBase>();
  params.addRequiredParam<Real>("depth", "The dimension of plate fuel in the third direction, m");
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
HeatStructurePlate::getUnitPerimeter(const MooseEnum & side) const
{
  if (side == "top")
    return _depth;
  else if (side == "bottom")
    return _depth;
  else
    mooseError(name(), ": The heat structure side value '", side, "' is invalid.");
}
