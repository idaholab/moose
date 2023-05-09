//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "OneDHeatFluxBase.h"
#include "HeatFluxFromHeatStructureBaseUserObject.h"
#include "HeatConductionModel.h"
#include "Assembly.h"

InputParameters
OneDHeatFluxBase::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addRequiredParam<UserObjectName>(
      "q_uo", "The name of the user object that computed the heat flux");
  return params;
}

OneDHeatFluxBase::OneDHeatFluxBase(const InputParameters & parameters)
  : Kernel(parameters),
    _phi_neighbor(_assembly.phiNeighbor(_var)),
    _q_uo(getUserObject<HeatFluxFromHeatStructureBaseUserObject>("q_uo"))
{
}

void
OneDHeatFluxBase::computeJacobian()
{
  Kernel::computeJacobian();
}

void
OneDHeatFluxBase::computeOffDiagJacobian(const unsigned int jvar_num)
{
  Kernel::computeOffDiagJacobian(jvar_num);

  if (jvar_num == _var.number())
  {
    // when doing the diagonal part, also take care of the off-diag jacobian
    // wrt the heat structure side
    std::vector<dof_id_type> idofs = _var.dofIndices();

    const dof_id_type & hs_elem_id = _q_uo.getNearestElem(_current_elem->id());
    const Elem * neighbor = _mesh.elemPtr(hs_elem_id);

    _assembly.setCurrentNeighborSubdomainID(neighbor->subdomain_id());
    _assembly.reinitNeighborAtPhysical(neighbor, _q_point.stdVector());

    std::vector<std::string> var_names = {HeatConductionModel::TEMPERATURE};
    for (std::size_t i = 0; i < var_names.size(); i++)
    {
      MooseVariableFEBase & jvar = _fe_problem.getVariable(_tid, var_names[i]);
      unsigned int jvar_num = jvar.number();
      jvar.prepareNeighbor();
      _assembly.copyNeighborShapes(jvar_num);

      auto & jdofs = jvar.dofIndicesNeighbor();
      DenseMatrix<Number> Ke(_test.size(), jvar.phiNeighborSize());
      for (_qp = 0; _qp < _qrule->n_points(); _qp++)
        for (_i = 0; _i < _test.size(); _i++)
          for (_j = 0; _j < jvar.phiNeighborSize(); _j++)
            Ke(_i, _j) += _JxW[_qp] * _coord[_qp] * computeQpOffDiagJacobianNeighbor(jvar_num);

      addJacobian(_assembly, Ke, idofs, jdofs, _var.scalingFactor());
    }
  }
}
