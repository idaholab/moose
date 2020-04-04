//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementJacobianDamper.h"
#include "FEProblem.h"
#include "MooseMesh.h"
#include "DisplacedProblem.h"
#include "Assembly.h"

#include "libmesh/quadrature.h" // _qrule

registerMooseObject("TensorMechanicsApp", ElementJacobianDamper);

InputParameters
ElementJacobianDamper::validParams()
{
  InputParameters params = GeneralDamper::validParams();
  params.addClassDescription("Damper that limits the change in element Jacobians");
  params.addParam<std::vector<VariableName>>("displacements",
                                             "The nonlinear displacement variables");
  params.addParam<Real>(
      "max_increment",
      0.1,
      "The maximum permissible relative increment in the Jacobian per Newton iteration");
  params.set<bool>("use_displaced_mesh") = true;
  return params;
}

ElementJacobianDamper::ElementJacobianDamper(const InputParameters & parameters)
  : GeneralDamper(parameters),
    _tid(parameters.get<THREAD_ID>("_tid")),
    _assembly(_subproblem.assembly(_tid)),
    _qrule(_assembly.qRule()),
    _JxW(_assembly.JxW()),
    _fe_problem(*parameters.getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _displaced_problem(_fe_problem.getDisplacedProblem()),
    _max_jacobian_diff(parameters.get<Real>("max_increment"))
{
  if (_displaced_problem == NULL)
    mooseError("ElementJacobianDamper: Must use displaced problem");

  _mesh = &_displaced_problem->mesh();

  const std::vector<VariableName> & nl_vnames(getParam<std::vector<VariableName>>("displacements"));
  _ndisp = nl_vnames.size();

  for (unsigned int i = 0; i < _ndisp; ++i)
  {
    _disp_var.push_back(&_sys.getFieldVariable<Real>(_tid, nl_vnames[i]));
    _disp_incr.push_back(_disp_var.back()->increment());
  }
}

void
ElementJacobianDamper::initialSetup()
{
}

Real
ElementJacobianDamper::computeDamping(const NumericVector<Number> & /* solution */,
                                      const NumericVector<Number> & update)
{
  // Maximum difference in the Jacobian for this Newton iteration
  Real max_difference = 0.0;
  MooseArray<Real> JxW_displaced;

  // Vector for storing the original node coordinates
  std::vector<Point> point_copies;

  // Loop over elements in the mesh
  for (auto & current_elem : _mesh->getMesh().active_local_element_ptr_range())
  {
    point_copies.clear();
    point_copies.reserve(current_elem->n_nodes());

    // Displace nodes with current Newton increment
    for (unsigned int i = 0; i < current_elem->n_nodes(); ++i)
    {
      Node & displaced_node = current_elem->node_ref(i);

      point_copies.push_back(displaced_node);

      for (unsigned int j = 0; j < _ndisp; ++j)
      {
        dof_id_type disp_dof_num =
            displaced_node.dof_number(_sys.number(), _disp_var[j]->number(), 0);
        displaced_node(j) += update(disp_dof_num);
      }
    }

    // Reinit element to compute Jacobian of displaced element
    _assembly.reinit(current_elem);
    JxW_displaced = _JxW;

    // Un-displace nodes
    for (unsigned int i = 0; i < current_elem->n_nodes(); ++i)
    {
      Node & displaced_node = current_elem->node_ref(i);

      for (unsigned int j = 0; j < _ndisp; ++j)
        displaced_node(j) = point_copies[i](j);
    }

    // Reinit element to compute Jacobian before displacement
    _assembly.reinit(current_elem);

    for (unsigned int qp = 0; qp < _qrule->n_points(); ++qp)
    {
      Real diff = std::abs(JxW_displaced[qp] - _JxW[qp]) / _JxW[qp];
      if (diff > max_difference)
        max_difference = diff;
    }

    JxW_displaced.release();
  }

  _communicator.max(max_difference);

  if (max_difference > _max_jacobian_diff)
    return _max_jacobian_diff / max_difference;

  return 1.0;
}
