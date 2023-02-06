//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HeatStructureBase.h"
#include "SolidMaterialProperties.h"
#include "ConstantFunction.h"
#include "THMEnums.h"

const std::map<std::string, HeatStructureSideType> HeatStructureBase::_side_type_to_enum{
    {"INNER", HeatStructureSideType::INNER},
    {"OUTER", HeatStructureSideType::OUTER},
    {"START", HeatStructureSideType::START},
    {"END", HeatStructureSideType::END}};

MooseEnum
HeatStructureBase::getSideType(const std::string & name)
{
  return THM::getMooseEnum<HeatStructureSideType>(name, _side_type_to_enum);
}

template <>
HeatStructureSideType
THM::stringToEnum(const std::string & s)
{
  return stringToEnum<HeatStructureSideType>(s, HeatStructureBase::_side_type_to_enum);
}

InputParameters
HeatStructureBase::validParams()
{
  InputParameters params = Component2D::validParams();
  params += HeatStructureInterface::validParams();
  return params;
}

HeatStructureBase::HeatStructureBase(const InputParameters & params)
  : Component2D(params), HeatStructureInterface(this), _connected_to_flow_channel(false)
{
}

void
HeatStructureBase::init()
{
  Component2D::init();
  HeatStructureInterface::init();
}

void
HeatStructureBase::check() const
{
  Component2D::check();
  HeatStructureInterface::check();
}

const unsigned int &
HeatStructureBase::getIndexFromName(const std::string & name) const
{
  return _name_index.at(name);
}

bool
HeatStructureBase::usingSecondOrderMesh() const
{
  return HeatConductionModel::feType().order == SECOND;
}

void
HeatStructureBase::addVariables()
{
  HeatStructureInterface::addVariables();
}

void
HeatStructureBase::addMooseObjects()
{
  HeatStructureInterface::addMooseObjects();

  if (isParamValid("materials"))
  {
    _hc_model->addMaterials();

    for (unsigned int i = 0; i < _number_of_hs; i++)
    {
      const SolidMaterialProperties & smp =
          getTHMProblem().getUserObject<SolidMaterialProperties>(_material_names[i]);

      Component * comp = (_parent != nullptr) ? _parent : this;
      // if the values were given as constant, allow them to be controlled
      const ConstantFunction * k_fn = dynamic_cast<const ConstantFunction *>(&smp.getKFunction());
      if (k_fn != nullptr)
        comp->connectObject(k_fn->parameters(), k_fn->name(), "k", "value");

      const ConstantFunction * cp_fn = dynamic_cast<const ConstantFunction *>(&smp.getCpFunction());
      if (cp_fn != nullptr)
        comp->connectObject(cp_fn->parameters(), cp_fn->name(), "cp", "value");

      const ConstantFunction * rho_fn =
          dynamic_cast<const ConstantFunction *>(&smp.getRhoFunction());
      if (rho_fn != nullptr)
        comp->connectObject(rho_fn->parameters(), rho_fn->name(), "rho", "value");
    }
  }
}

const std::vector<unsigned int> &
HeatStructureBase::getSideNodeIds(const std::string & name) const
{
  checkSetupStatus(MESH_PREPARED);

  return _side_heat_node_ids.at(name);
}

const std::vector<unsigned int> &
HeatStructureBase::getOuterNodeIds() const
{
  checkSetupStatus(MESH_PREPARED);

  return _outer_heat_node_ids;
}

const std::vector<BoundaryName> &
HeatStructureBase::getOuterBoundaryNames() const
{
  checkSetupStatus(MESH_PREPARED);

  return _boundary_names_outer;
}

const std::vector<BoundaryName> &
HeatStructureBase::getInnerBoundaryNames() const
{
  checkSetupStatus(MESH_PREPARED);

  return _boundary_names_inner;
}

const std::vector<BoundaryName> &
HeatStructureBase::getStartBoundaryNames() const
{
  checkSetupStatus(MESH_PREPARED);

  return _boundary_names_start;
}

const std::vector<BoundaryName> &
HeatStructureBase::getEndBoundaryNames() const
{
  checkSetupStatus(MESH_PREPARED);

  return _boundary_names_end;
}

const std::vector<std::tuple<dof_id_type, unsigned short int>> &
HeatStructureBase::getBoundaryInfo(const HeatStructureSideType & side) const
{
  switch (side)
  {
    case HeatStructureSideType::INNER:
      return getBoundaryInfo(_boundary_names_inner[0]);
    case HeatStructureSideType::OUTER:
      return getBoundaryInfo(_boundary_names_outer[0]);
    case HeatStructureSideType::START:
      return getBoundaryInfo(_boundary_names_start[0]);
    case HeatStructureSideType::END:
      return getBoundaryInfo(_boundary_names_end[0]);
  }

  mooseError(name(), ": Unknown value of 'side' parameter.");
}

bool
HeatStructureBase::hasHeatStructureSideType(const BoundaryName & boundary_name) const
{
  return isBoundaryInVector(boundary_name, _boundary_names_inner) ||
         isBoundaryInVector(boundary_name, _boundary_names_axial_inner) ||
         isBoundaryInVector(boundary_name, _boundary_names_outer) ||
         isBoundaryInVector(boundary_name, _boundary_names_axial_outer) ||
         isBoundaryInVector(boundary_name, _boundary_names_start) ||
         isBoundaryInVector(boundary_name, _boundary_names_radial_start) ||
         isBoundaryInVector(boundary_name, _boundary_names_end) ||
         isBoundaryInVector(boundary_name, _boundary_names_radial_end);
}

HeatStructureSideType
HeatStructureBase::getHeatStructureSideType(const BoundaryName & boundary_name) const
{
  if (isBoundaryInVector(boundary_name, _boundary_names_inner) ||
      isBoundaryInVector(boundary_name, _boundary_names_axial_inner))
    return HeatStructureSideType::INNER;
  else if (isBoundaryInVector(boundary_name, _boundary_names_outer) ||
           isBoundaryInVector(boundary_name, _boundary_names_axial_outer))
    return HeatStructureSideType::OUTER;
  else if (isBoundaryInVector(boundary_name, _boundary_names_start) ||
           isBoundaryInVector(boundary_name, _boundary_names_radial_start))
    return HeatStructureSideType::START;
  else if (isBoundaryInVector(boundary_name, _boundary_names_end) ||
           isBoundaryInVector(boundary_name, _boundary_names_radial_end))
    return HeatStructureSideType::END;
  else if (isBoundaryInVector(boundary_name, _boundary_names_interior_axial_per_radial_section) ||
           isBoundaryInVector(boundary_name, _boundary_names_inner_radial))
    mooseError("The boundary '", boundary_name, "' is an interior boundary.");
  else
    mooseError("No heat structure side type was found for the boundary '", boundary_name, "'.");
}
