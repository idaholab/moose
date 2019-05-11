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

template <>
InputParameters
validParams<InterfaceMaterial>()
{
  InputParameters params = validParams<MaterialBase>();
  params += validParams<BoundaryRestrictable>();
  params += validParams<TwoMaterialPropertyInterface>();
  return params;
}

InterfaceMaterial::InterfaceMaterial(const InputParameters & parameters)
  : MaterialBase(parameters),
    // BoundaryRestrictable(this, Moose::EMPTY_BLOCK_IDS, false), // false for being _not_ nodal
    NeighborCoupleable(this, false, false),
    TwoMaterialPropertyInterface(this, Moose::EMPTY_BLOCK_IDS, boundaryIDs()),
    _bnd(_material_data_type == Moose::INTERFACE_MATERIAL_DATA),
    // _neighbor(tr_material_data_type == Moose::NEIGHBOR_MATERIAL_DATA),
    _neighbor(true),
    _q_point(_assembly.qPointsFace()),
    _qrule(_assembly.qRuleFace()),
    _JxW(_assembly.JxWFace()),
    _current_elem(_neighbor ? _assembly.neighbor() : _assembly.elem()),
    _current_subdomain_id(_neighbor ? _assembly.currentNeighborSubdomainID()
                                    : _assembly.currentSubdomainID()),
    _current_side(_neighbor ? _assembly.neighborSide() : _assembly.side())
{
  // Fill in the MooseVariable dependencies
  const std::vector<MooseVariableFEBase *> & coupled_vars = getCoupledMooseVars();
  for (const auto & var : coupled_vars)
    addMooseVariableDependency(var);
}

void
InterfaceMaterial::computeProperties()
{

  // Reference to *all* the MaterialProperties in the MaterialData object, not
  // just the ones for this Material.

  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
    computeQpProperties();
  copyDualNumbersToValues();
}
