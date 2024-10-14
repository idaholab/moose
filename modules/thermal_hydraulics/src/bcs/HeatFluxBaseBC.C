//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HeatFluxBaseBC.h"
#include "HeatFluxFromHeatStructureBaseUserObject.h"
#include "THMIndicesVACE.h"
#include "Assembly.h"
#include "NonlinearSystemBase.h"

InputParameters
HeatFluxBaseBC::validParams()
{
  InputParameters params = IntegratedBC::validParams();
  params.addRequiredParam<UserObjectName>(
      "q_uo", "The name of the user object that computes the heat flux");
  params.addRequiredParam<Real>("P_hs_unit", "Perimeter of a single unit of heat structure");
  params.addRequiredParam<unsigned int>("n_unit", "Number of units of heat structure");
  params.addRequiredParam<bool>("hs_coord_system_is_cylindrical",
                                "Is the heat structure coordinate system cylindrical?");
  params.addClassDescription("Base class for heat flux boundary conditions");
  return params;
}

HeatFluxBaseBC::HeatFluxBaseBC(const InputParameters & parameters)
  : IntegratedBC(parameters),
    _phi_neighbor(_assembly.phiNeighbor(_var)),
    _q_uo(getUserObject<HeatFluxFromHeatStructureBaseUserObject>("q_uo")),
    _P_hs_unit(getParam<Real>("P_hs_unit")),
    _n_unit(getParam<unsigned int>("n_unit")),
    _hs_coord_system_is_cylindrical(getParam<bool>("hs_coord_system_is_cylindrical")),
    _hs_coord(_hs_coord_system_is_cylindrical ? _P_hs_unit : 1.0),
    _hs_scale(-_hs_coord / (_n_unit * _P_hs_unit))
{
}

void
HeatFluxBaseBC::initialSetup()
{
  _off_diag_var_nums = getOffDiagVariableNumbers();
}

void
HeatFluxBaseBC::computeJacobian()
{
  IntegratedBC::computeJacobian();
}

void
HeatFluxBaseBC::computeOffDiagJacobian(const unsigned int jvar_num)
{
  IntegratedBC::computeOffDiagJacobian(jvar_num);

  if (jvar_num == _var.number())
  {
    // when doing the diagonal part, also take care of the off-diag jacobian
    // wrt the heat structure side
    std::vector<dof_id_type> idofs = _var.dofIndices();

    const dof_id_type & pipe_elem_id = _q_uo.getNearestElem(_current_elem->id());
    const Elem * neighbor = _mesh.elemPtr(pipe_elem_id);

    _assembly.setCurrentNeighborSubdomainID(neighbor->subdomain_id());
    _assembly.reinitNeighborAtPhysical(neighbor, _q_point.stdVector());

    for (std::size_t i = 0; i < _off_diag_var_nums.size(); i++)
    {
      unsigned int jvar_num = _off_diag_var_nums[i];
      MooseVariableFEBase & jvar =
          _fe_problem.getNonlinearSystemBase(_sys.number()).getVariable(_tid, jvar_num);
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
