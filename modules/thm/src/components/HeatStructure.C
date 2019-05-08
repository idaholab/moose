#include "HeatStructure.h"
#include "HeatConductionModel.h"

const std::map<std::string, HeatStructure::EHeatStructureType> HeatStructure::_hs_type_to_enum{
    {"PLATE", PLATE}, {"CYLINDER", CYLINDER}};

MooseEnum
HeatStructure::getHeatStructureType(const std::string & name)
{
  return THM::getMooseEnum<EHeatStructureType>(name, _hs_type_to_enum);
}

template <>
HeatStructure::EHeatStructureType
THM::stringToEnum(const std::string & s)
{
  return stringToEnum<HeatStructure::EHeatStructureType>(s, HeatStructure::_hs_type_to_enum);
}

registerMooseObject("THMApp", HeatStructure);

template <>
InputParameters
validParams<HeatStructure>()
{
  InputParameters params = validParams<GeometricalComponent>();
  params.addPrivateParam<std::string>("component_type", "heat_struct");
  params.addParam<unsigned int>("dim", 2, "Dimension of the geometry.");
  params.addRequiredParam<MooseEnum>(
      "hs_type", HeatStructure::getHeatStructureType(), "Geometry type of the heat structure");
  params.addParam<Real>("axial_offset", 0., "Axial offset for the undisplaced mesh");
  params.addParam<Real>("depth", 0., "The dimension of plate fuel in the third direction, m");
  params.addParam<FunctionName>("initial_T", "Initial temperature");
  params.addRequiredParam<std::vector<std::string>>("names", "User given heat structure names");
  params.addRequiredParam<std::vector<Real>>("widths", "Width of each heat structure");
  params.addRequiredParam<std::vector<unsigned int>>("n_part_elems",
                                                     "Number of elements of each heat structure");
  params.addRequiredParam<std::vector<std::string>>("materials",
                                                    "Material names to be used in heat structures");
  params.addParam<Real>("num_rods", 1., "The number of rods represented by this heat structure.");
  return params;
}

HeatStructure::HeatStructure(const InputParameters & params) : GeometricalComponent(params)
{
  logError(
      "HeatStructure component is deprecated. For cylindrical heat structures use 'type = "
      "HeatStructureCylindrical' and for plate heat structures use 'type = HeatStructurePlate'.");
}

void
HeatStructure::buildMesh()
{
}

void
HeatStructure::init()
{
}

bool
HeatStructure::usingSecondOrderMesh() const
{
  return HeatConductionModel::feType().order == SECOND;
}
