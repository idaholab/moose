//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MortarUserObjectThread.h"
#include "FEProblemBase.h"
#include "SubProblem.h"
#include "Assembly.h"
#include "AutomaticMortarGeneration.h"
#include "MooseMesh.h"
#include "Assembly.h"
#include "MortarUtils.h"
#include "MaterialBase.h"
#include "MortarUserObject.h"

#include "libmesh/fe_base.h"
#include "libmesh/quadrature.h"
#include "libmesh/elem.h"
#include "libmesh/point.h"
#include "libmesh/mesh_base.h"

MortarUserObjectThread::MortarUserObjectThread(
    std::vector<MortarUserObject *> & mortar_user_objects,
    const AutomaticMortarGeneration & amg,
    SubProblem & subproblem,
    FEProblemBase & fe_problem,
    bool displaced,
    Assembly & assembly)
  : _mortar_user_objects(mortar_user_objects),
    _amg(amg),
    _subproblem(subproblem),
    _fe_problem(fe_problem),
    _displaced(displaced),
    _assembly(assembly)
{
  Moose::Mortar::setupMortarMaterials(_mortar_user_objects,
                                      _fe_problem,
                                      _amg,
                                      0,
                                      _secondary_ip_sub_to_mats,
                                      _primary_ip_sub_to_mats,
                                      _secondary_boundary_mats);
}

void
MortarUserObjectThread::operator()()
{
  const auto & secondary_elems_to_mortar_segments = _amg.secondariesToMortarSegments();
  typedef decltype(secondary_elems_to_mortar_segments.begin()) it_type;

  std::vector<it_type> iterators;
  for (auto it = secondary_elems_to_mortar_segments.begin();
       it != secondary_elems_to_mortar_segments.end();
       ++it)
  {
    auto * const secondary_elem = _subproblem.mesh().getMesh().query_elem_ptr(it->first);

    if (secondary_elem && secondary_elem->processor_id() == _subproblem.processor_id() &&
        !it->second.empty())
    {
      // This is local and the mortar segment set isn't empty, so include
      iterators.push_back(it);
      mooseAssert(secondary_elem->active(),
                  "We loop over active elements when building the mortar segment mesh, so we golly "
                  "well hope this is active.");
    }
  }

  auto act_functor = [this]()
  {
    for (auto * const mc : _mortar_user_objects)
    {
      mc->setNormals();
      mc->execute();
    }
  };

  Moose::Mortar::loopOverMortarSegments(iterators,
                                        _assembly,
                                        _subproblem,
                                        _fe_problem,
                                        _amg,
                                        _displaced,
                                        _mortar_user_objects,
                                        0,
                                        _secondary_ip_sub_to_mats,
                                        _primary_ip_sub_to_mats,
                                        _secondary_boundary_mats,
                                        act_functor,
                                        /*reinit_mortar_user_objects=*/false);
}
