/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "IntegratedBCBase.h"
#include "Assembly.h"

template <>
InputParameters
validParams<IntegratedBCBase>()
{
  InputParameters params = validParams<BoundaryCondition>();
  params += validParams<RandomInterface>();
  params += validParams<MaterialPropertyInterface>();

  params.addParam<std::vector<AuxVariableName>>(
      "save_in",
      "The name of auxiliary variables to save this BC's residual contributions to.  "
      "Everything about that variable must match everything about this variable (the "
      "type, what blocks it's on, etc.)");
  params.addParam<std::vector<AuxVariableName>>(
      "diag_save_in",
      "The name of auxiliary variables to save this BC's diagonal jacobian "
      "contributions to.  Everything about that variable must match everything "
      "about this variable (the type, what blocks it's on, etc.)");

  params.addParamNamesToGroup("diag_save_in save_in", "Advanced");

  // Integrated BCs always rely on Boundary MaterialData
  params.set<Moose::MaterialDataType>("_material_data_type") = Moose::BOUNDARY_MATERIAL_DATA;

  return params;
}

IntegratedBCBase::IntegratedBCBase(const InputParameters & parameters)
  : BoundaryCondition(parameters, false), // False is because this is NOT nodal
    RandomInterface(parameters, _fe_problem, _tid, false),
    CoupleableMooseVariableDependencyIntermediateInterface(this, false),
    MaterialPropertyInterface(this, Moose::EMPTY_BLOCK_IDS, boundaryIDs()),
    _current_elem(_assembly.elem()),
    _current_elem_volume(_assembly.elemVolume()),
    _current_side(_assembly.side()),
    _current_side_elem(_assembly.sideElem()),
    _current_side_volume(_assembly.sideElemVolume()),
    _qrule(_assembly.qRuleFace()),
    _q_point(_assembly.qPointsFace()),
    _JxW(_assembly.JxWFace()),
    _coord(_assembly.coordTransformation()),
    _save_in_strings(parameters.get<std::vector<AuxVariableName>>("save_in")),
    _diag_save_in_strings(parameters.get<std::vector<AuxVariableName>>("diag_save_in"))
{
}

IntegratedBCBase::~IntegratedBCBase() {}
