//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SingularShapeTensorEliminatorUserObjectPD.h"
#include "AuxiliarySystem.h"

registerMooseObject("PeridynamicsApp", SingularShapeTensorEliminatorUserObjectPD);

InputParameters
SingularShapeTensorEliminatorUserObjectPD::validParams()
{
  InputParameters params = GeneralUserObjectBasePD::validParams();
  params.addClassDescription("UserObject to eliminate the existance of singular shape tensor");

  return params;
}

SingularShapeTensorEliminatorUserObjectPD::SingularShapeTensorEliminatorUserObjectPD(
    const InputParameters & parameters)
  : GeneralUserObjectBasePD(parameters)
{
}

void
SingularShapeTensorEliminatorUserObjectPD::initialize()
{
  _aux.solution().close();
}

void
SingularShapeTensorEliminatorUserObjectPD::execute()
{
  bool converged = false;

  // Loop through the active local elements to check the shape tensor singularity
  auto first_elem = _mesh.getMesh().active_local_elements_begin();
  auto last_elem = _mesh.getMesh().active_local_elements_end();

  unsigned int singularity_count, loop_count = 0;
  bool should_print = true; // for printing purpose only

  while (!converged)
  {
    singularity_count = 0;
    for (auto elem = first_elem; elem != last_elem; ++elem)
    {
      // shape tensor singularity check only applies to intact Edge2 elems
      if ((*elem)->type() == 0 && _bond_status_var->getElementalValue(*elem) > 0.5)
      {
        if (checkShapeTensorSingularity(*elem))
        {
          dof_id_type dof = (*elem)->dof_number(_aux.number(), _bond_status_var->number(), 0);
          _aux.solution().set(dof, 0); // treat bonds with singular shape tensor as broken

          singularity_count++;
        }
      }
    }

    gatherSum(singularity_count); // gather the value across processors

    if (singularity_count == 0)
      converged = true;
    else // sync aux across processors
    {
      _aux.solution().close();
      _aux.update();
    }

    if (singularity_count > 0 && should_print) // print once
    {
      _console << COLOR_MAGENTA << " Singular shape tensor detected! Elimination in process ... "
               << COLOR_DEFAULT << std::endl;
      should_print = false;
    }

    loop_count++;
    if ((loop_count == 1 && singularity_count != 0) || loop_count > 1)
      _console << COLOR_MAGENTA << "     Loop: " << loop_count
               << ", Singularities: " << singularity_count << COLOR_DEFAULT << std::endl;
  }

  if (loop_count > 1)
    _console << COLOR_MAGENTA << " Elimination done!" << COLOR_DEFAULT << std::endl;
}

void
SingularShapeTensorEliminatorUserObjectPD::finalize()
{
  _aux.solution().close();
}

bool
SingularShapeTensorEliminatorUserObjectPD::checkShapeTensorSingularity(const Elem * elem)
{
  bool singular = false;

  RankTwoTensor shape_tensor;
  Real vol_nb, weight_nb, horizon_nd;
  RealGradient origin_vec_nb;
  for (unsigned int nd = 0; nd < _nnodes; ++nd)
  {
    std::vector<dof_id_type> neighbors = _pdmesh.getNeighbors(elem->node_id(nd));
    std::vector<dof_id_type> bonds =
        _pdmesh.getBonds(elem->node_id(nd)); // potentially includes ghosted elems
    horizon_nd = _pdmesh.getHorizon(elem->node_id(nd));

    shape_tensor.zero();
    if (_dim == 2)
      shape_tensor(2, 2) = 1.0;

    for (unsigned int nb = 0; nb < neighbors.size(); ++nb)
      if (_bond_status_var->getElementalValue(_pdmesh.elemPtr(bonds[nb])) > 0.5)
      {
        vol_nb = _pdmesh.getNodeVolume(neighbors[nb]);
        origin_vec_nb = *_pdmesh.nodePtr(neighbors[nb]) - *elem->node_ptr(nd);
        weight_nb = horizon_nd / origin_vec_nb.norm();

        for (unsigned int k = 0; k < _dim; ++k)
          for (unsigned int l = 0; l < _dim; ++l)
            shape_tensor(k, l) += weight_nb * origin_vec_nb(k) * origin_vec_nb(l) * vol_nb;
      }

    if (shape_tensor.det() == 0.)
      singular = true;
  }

  return singular;
}
