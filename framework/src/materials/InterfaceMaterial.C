//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "InterfaceMaterial.h"

InputParameters
InterfaceMaterial::validParams()
{

  InputParameters params = MaterialBase::validParams();
  params += TwoMaterialPropertyInterface::validParams();
  params.set<bool>("_interface") = true;
  params.set<Moose::MaterialDataType>("_material_data_type") = Moose::INTERFACE_MATERIAL_DATA;

  // Interface materials always need one layer of ghosting to be safe
  params.addRelationshipManager("ElementSideNeighborLayers",
                                Moose::RelationshipManagerType::GEOMETRIC |
                                    Moose::RelationshipManagerType::ALGEBRAIC);
  return params;
}

InterfaceMaterial::InterfaceMaterial(const InputParameters & parameters)
  : MaterialBase(parameters),
    NeighborCoupleable(this, false, false),
    TwoMaterialPropertyInterface(this, blockIDs(), boundaryIDs()),
    _q_point(_assembly.qPointsFace()),
    _qrule(_assembly.qRuleFace()),
    _JxW(_assembly.JxWFace()),
    _current_elem(_assembly.elem()),
    _neighbor_elem(_assembly.neighbor()),
    _current_side(_assembly.side()),
    _neighbor_side(_assembly.neighborSide())
{
  // Fill in the MooseVariable dependencies
  const std::vector<MooseVariableFEBase *> & coupled_vars = getCoupledMooseVars();
  for (const auto & var : coupled_vars)
    addMooseVariableDependency(var);
}

void
InterfaceMaterial::computeProperties()
{
  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
    computeQpProperties();
}
