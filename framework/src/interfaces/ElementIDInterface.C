//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementIDInterface.h"

#include "InputParameters.h"
#include "MooseObject.h"
#include "MooseApp.h"
#include "ActionWarehouse.h"
#include "MooseMesh.h"
#include "SubProblem.h"
#include "Assembly.h"

#include "libmesh/mesh_base.h"

ElementIDInterface::ElementIDInterface(const MooseObject * moose_object)
  : _obj_parameters(moose_object->parameters()),
    _mesh(moose_object->getMooseApp().actionWarehouse().mesh())
{
}

unsigned int
ElementIDInterface::getElementIDIndex(const std::string & id_parameter_name,
                                      unsigned int comp) const
{
  auto & p = _obj_parameters.get<std::vector<ExtraElementIDName>>(id_parameter_name);
  if (comp >= p.size())
    mooseError(id_parameter_name, " does not have enough integer names");

  if (!_mesh.get())
    mooseError("Mesh is not available for getting element integers");

  auto & mesh_base = _mesh->getMesh();

  if (!mesh_base.has_elem_integer(p[comp]))
    mooseError("Mesh does not have an element integer names as ", p[comp]);

  auto id = mesh_base.get_elem_integer_index(p[comp]);

  return id;
}

const dof_id_type &
ElementIDInterface::getElementID(const std::string & id_parameter_name, unsigned int comp) const
{
  auto id = getElementIDIndex(id_parameter_name, comp);

  auto & _subproblem = *_obj_parameters.getCheckedPointerParam<SubProblem *>("_subproblem");

  auto tid = _obj_parameters.get<THREAD_ID>("_tid");

  auto & assembly = _subproblem.assembly(tid);

  return assembly.extraElemID(id);
}
