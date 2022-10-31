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

InputParameters
ElementIDInterface::validParams()
{
  return emptyInputParameters();
}

ElementIDInterface::ElementIDInterface(const MooseObject * moose_object)
  : _obj_parameters(moose_object->parameters()),
    _id_mesh(moose_object->getMooseApp().actionWarehouse().mesh()),
    _ei_name(moose_object->name())
{
}

unsigned int
ElementIDInterface::getElementIDIndex(const std::string & id_parameter_name,
                                      unsigned int comp) const
{
  auto & p = _obj_parameters.get<std::vector<ExtraElementIDName>>(id_parameter_name);
  if (comp >= p.size())
    mooseError(id_parameter_name, " does not have enough integer names");

  return getElementIDIndexByName(p[comp]);
}

unsigned int
ElementIDInterface::getElementIDIndexByName(const std::string & id_name) const
{
  if (!_id_mesh.get())
    mooseError("Mesh is not available for getting element integers");

  auto & mesh_base = _id_mesh->getMesh();

  if (id_name == "subdomain_id")
  {
    if (mesh_base.has_elem_integer(id_name))
      mooseError("MOOSE does not allow 'subdomain_id' element integer in a mesh. 'subdomain_id' is "
                 "reserved for element subdomain ID");
    return mesh_base.n_elem_integers();
  }

  if (!mesh_base.has_elem_integer(id_name))
    mooseError(
        "Mesh does not have an element integer names as ", id_name, " but required by ", _ei_name);

  auto id = mesh_base.get_elem_integer_index(id_name);

  return id;
}

const dof_id_type &
ElementIDInterface::getElementID(const std::string & id_parameter_name, unsigned int comp) const
{
  auto id = getElementIDIndex(id_parameter_name, comp);

  auto & _subproblem = *_obj_parameters.getCheckedPointerParam<SubProblem *>("_subproblem");

  auto tid = _obj_parameters.get<THREAD_ID>("_tid");

  auto & assembly = _subproblem.assembly(tid, 0);

  return assembly.extraElemID(id);
}

const dof_id_type &
ElementIDInterface::getElementIDNeighbor(const std::string & id_parameter_name,
                                         unsigned int comp) const
{
  auto id = getElementIDIndex(id_parameter_name, comp);

  auto & _subproblem = *_obj_parameters.getCheckedPointerParam<SubProblem *>("_subproblem");

  auto tid = _obj_parameters.get<THREAD_ID>("_tid");

  auto & assembly = _subproblem.assembly(tid, 0);

  return assembly.extraElemIDNeighbor(id);
}

const dof_id_type &
ElementIDInterface::getElementIDByName(const std::string & id_name) const
{
  auto id = getElementIDIndexByName(id_name);

  auto & _subproblem = *_obj_parameters.getCheckedPointerParam<SubProblem *>("_subproblem");

  auto tid = _obj_parameters.get<THREAD_ID>("_tid");

  auto & assembly = _subproblem.assembly(tid, 0);

  return assembly.extraElemID(id);
}

const dof_id_type &
ElementIDInterface::getElementIDNeighborByName(const std::string & id_name) const
{
  auto id = getElementIDIndexByName(id_name);

  auto & _subproblem = *_obj_parameters.getCheckedPointerParam<SubProblem *>("_subproblem");

  auto tid = _obj_parameters.get<THREAD_ID>("_tid");

  auto & assembly = _subproblem.assembly(tid, 0);

  return assembly.extraElemIDNeighbor(id);
}
