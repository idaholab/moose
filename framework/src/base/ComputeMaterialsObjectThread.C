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
#include "ComputeMaterialsObjectThread.h"

#include "NonlinearSystem.h"
#include "Problem.h"
#include "FEProblem.h"
#include "MaterialWarehouse.h"
#include "MaterialPropertyStorage.h"
#include "MaterialData.h"
#include "Assembly.h"
#include "AuxKernel.h"

// libmesh includes
#include "libmesh/threads.h"

ComputeMaterialsObjectThread::ComputeMaterialsObjectThread(FEProblem & fe_problem, NonlinearSystem & sys, std::vector<MaterialData *> & material_data,
                                                           std::vector<MaterialData *> & bnd_material_data, MaterialPropertyStorage & material_props,
                                                           MaterialPropertyStorage & bnd_material_props, std::vector<MaterialWarehouse> & materials,
                                                           std::vector<Assembly *> & assembly) :
ThreadedElementLoop<ConstElemRange>(fe_problem, sys),
  _fe_problem(fe_problem),
  _sys(sys),
  _material_data(material_data),
  _bnd_material_data(bnd_material_data),
  _material_props(material_props),
  _bnd_material_props(bnd_material_props),
  _materials(materials),
  _assembly(assembly)
{
}

// Splitting Constructor
ComputeMaterialsObjectThread::ComputeMaterialsObjectThread(ComputeMaterialsObjectThread & x, Threads::split split) :
    ThreadedElementLoop<ConstElemRange>(x, split),
    _fe_problem(x._fe_problem),
    _sys(x._sys),
    _material_data(x._material_data),
    _bnd_material_data(x._bnd_material_data),
    _material_props(x._material_props),
    _bnd_material_props(x._bnd_material_props),
    _materials(x._materials),
    _assembly(x._assembly)
{
}

ComputeMaterialsObjectThread::~ComputeMaterialsObjectThread()
{
}

void
ComputeMaterialsObjectThread::subdomainChanged()
{
  mooseAssert(_materials[_tid].hasMaterials(_subdomain), "No materials on subdomain block");
  _fe_problem.subdomainSetup(_subdomain, _tid);
}

void
ComputeMaterialsObjectThread::onElement(const Elem *elem)
{
  _assembly[_tid]->reinit(elem);

  unsigned int n_points = _assembly[_tid]->qRule()->n_points();
  _material_props.initStatefulProps(*_material_data[_tid], _materials[_tid].getMaterials(_subdomain), n_points, *elem);
}

void
ComputeMaterialsObjectThread::onBoundary(const Elem *elem, unsigned int side, BoundaryID bnd_id)
{
//  if (!_sys.hasActiveIntegratedBCs(bnd_id, _tid) && !_sys.doingDG())
//    return;

  _fe_problem.setCurrentBoundaryID(bnd_id);
  _assembly[_tid]->reinit(elem, side);
  unsigned int n_points = _assembly[_tid]->qRuleFace()->n_points();
  _bnd_material_props.initStatefulProps(*_bnd_material_data[_tid], _materials[_tid].getFaceMaterials(_subdomain), n_points, *elem, side);
  _fe_problem.setCurrentBoundaryID(Moose::INVALID_BOUNDARY_ID);
}

void
ComputeMaterialsObjectThread::onInternalSide(const Elem *elem, unsigned int side)
{
  if (!_sys.doingDG())
    return;

  _assembly[_tid]->reinit(elem, side);
  unsigned int n_points = _assembly[_tid]->qRuleFace()->n_points();
  _bnd_material_props.initStatefulProps(*_bnd_material_data[_tid], _materials[_tid].getFaceMaterials(_subdomain), n_points, *elem, side);
}

void
ComputeMaterialsObjectThread::join(const ComputeMaterialsObjectThread & /*y*/)
{
}
