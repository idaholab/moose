//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Pressure.h"

#include "Assembly.h"
#include "Function.h"
#include "MooseError.h"
#include "FEProblemBase.h"

template <bool is_ad>
InputParameters
PressureBaseTempl<is_ad>::validParams()
{
  InputParameters params = PressureBaseParent<is_ad>::validParams();
  params.addDeprecatedParam<unsigned int>(
      "component", "The component for the pressure", "This parameter is no longer necessary");
  params.addParam<bool>("use_displaced_mesh", true, "Whether to use the displaced mesh.");
  return params;
}

template <bool is_ad>
InputParameters
PressureBaseTempl<is_ad>::actionParams()
{
  auto params = emptyInputParameters();
  params.addRequiredCoupledVar("displacements",
                               "The string of displacements suitable for the problem statement");
  return params;
}

template <bool is_ad>
PressureBaseTempl<is_ad>::PressureBaseTempl(const InputParameters & parameters)
  : PressureBaseParent<is_ad>(parameters),
    _ndisp(this->coupledComponents("displacements")),
    _component(
        [this, &parameters]()
        {
          for (unsigned int i = 0; i < _ndisp; ++i)
            _disp_var.push_back(this->coupled("displacements", i));

          for (unsigned int i = 0; i < _ndisp; ++i)
          {
            if (_var.number() == _disp_var[i])
            {
              if (parameters.isParamSetByUser("component") &&
                  i != this->template getParam<unsigned int>("component"))
                this->paramError(
                    "component",
                    "The component this BC is acting on is now inferred from the position "
                    "of the `variable` in the `displacements` variable list. The explicitly "
                    "specified component value is at odds with teh automatically inferred "
                    "value. The `component` parameter has been deprecated. Please double "
                    "check your input for potential mestakes.");
              return i;
            }
          }

          this->paramError("variable", "The BC variable should be a displacement variable.");
        }())
{
}

template <bool is_ad>
void
PressureBaseTempl<is_ad>::initialSetup()
{
  auto boundary_ids = this->boundaryIDs();
  std::set<SubdomainID> block_ids;
  for (auto bndry_id : boundary_ids)
  {
    auto bids = _mesh.getBoundaryConnectedBlocks(bndry_id);
    block_ids.insert(bids.begin(), bids.end());
  }

  if (block_ids.size())
    _coord_type = _fe_problem.getCoordSystem(*block_ids.begin());
  else
  {
    mooseInfo(
        "No connected blocks were found, the coordinate system type is obtained from the mesh.");
    _coord_type = _fe_problem.mesh().getUniqueCoordSystem();
  }
  for (auto blk_id : block_ids)
  {
    if (_coord_type != _fe_problem.getCoordSystem(blk_id))
      mooseError("The Pressure BC requires subdomains to have the same coordinate system.");
  }
}

template <bool is_ad>
GenericReal<is_ad>
PressureBaseTempl<is_ad>::computeQpResidual()
{
  return computePressure() * (_normals[_qp](_component) * _test[_i][_qp]);
}

PressureBase::PressureBase(const InputParameters & parameters)
  : PressureBaseTempl<false>(parameters),
    _q_dxi(nullptr),
    _q_deta(nullptr),
    _phi_dxi(nullptr),
    _phi_deta(nullptr),
    _use_displaced_mesh(this->template getParam<bool>("use_displaced_mesh")),
    _fe(libMesh::n_threads())
{
}

Real
PressureBase::computeFaceStiffness(const unsigned int local_j, const unsigned int coupled_component)
{
  //
  // Note that this approach will break down for shell elements, i.e.,
  //   topologically 2D elements in 3D space with pressure loads on
  //   the faces.
  //
  const Real phi_dxi = (*_phi_dxi)[local_j][_qp];
  const Real phi_deta = _phi_deta ? (*_phi_deta)[local_j][_qp] : 0;

  const RealGradient & dqdxi = (*_q_dxi)[_qp];
  const RealGradient out_of_plane(0, 0, 1);
  const RealGradient & dqdeta = _q_deta ? (*_q_deta)[_qp] : out_of_plane;
  // Here, b is dqdxi (cross) dqdeta
  // Then, normal is b/length(b)
  RealGradient b(dqdxi(1) * dqdeta(2) - dqdxi(2) * dqdeta(1),
                 dqdxi(2) * dqdeta(0) - dqdxi(0) * dqdeta(2),
                 dqdxi(0) * dqdeta(1) - dqdxi(1) * dqdeta(0));
  const Real inv_length = 1 / (b * _normals[_qp]);

  const unsigned int i = _component;
  const unsigned int j = coupled_component;

  // const int posneg[3][3] = {{0, -1, 1}, {1, 0, -1}, {-1, 1, 0}};
  const int posneg = 1 - (j + (2 - (i + 1) % 3)) % 3;

  // const unsigned int index[3][3] = {{0, 2, 1}, {2, 1, 0}, {1, 0, 2}};
  const unsigned int index = 2 - (j + (i + 2) % 3) % 3;

  const Real variation_b = posneg * (phi_deta * dqdxi(index) - phi_dxi * dqdeta(index));

  Real rz_term = 0;
  if (_coord_type == Moose::COORD_RZ && j == _subproblem.getAxisymmetricRadialCoord())
  {
    rz_term = _normals[_qp](i) * _phi[_j][_qp] / _q_point[_qp](0);
  }

  return computePressure() * _test[_i][_qp] * (inv_length * variation_b + rz_term);
}

Real
PressureBase::computeStiffness(const unsigned int coupled_component)
{
  if (_ndisp > 1)
  {
    const std::map<unsigned int, unsigned int>::iterator j_it = _node_map.find(_j);
    if (_test[_i][_qp] == 0 || j_it == _node_map.end())
      return 0;

    return computeFaceStiffness(j_it->second, coupled_component);
  }

  else if (_coord_type == Moose::COORD_RSPHERICAL)
  {
    return computePressure() * _normals[_qp](_component) * _test[_i][_qp] * _phi[_j][_qp] *
           (2 / _q_point[_qp](0));
  }

  if (_coord_type == Moose::COORD_RZ)
  {
    return computePressure() * _normals[_qp](_component) * _test[_i][_qp] * _phi[_j][_qp] /
           _q_point[_qp](0);
  }

  return 0;
}

Real
PressureBase::computeQpJacobian()
{
  if (_use_displaced_mesh)
    return computeStiffness(_component);

  return 0;
}

Real
PressureBase::computeQpOffDiagJacobian(const unsigned int jvar_num)
{
  if (_use_displaced_mesh)
    for (unsigned int j = 0; j < _ndisp; ++j)
      if (jvar_num == _disp_var[j])
        return computeStiffness(j);

  return 0;
}

void
PressureBase::precalculateQpJacobian()
{
  if (_ndisp == 1)
    return;

  if (_fe[_tid] == nullptr)
  {
    const unsigned int dim = _sys.mesh().dimension() - 1;
    QBase * const & qrule = _assembly.writeableQRuleFace();
    _fe[_tid] = FEBase::build(dim, _var.feType());
    _fe[_tid]->attach_quadrature_rule(qrule);
  }

  _q_dxi = &_fe[_tid]->get_dxyzdxi();
  _phi_dxi = &_fe[_tid]->get_dphidxi();
  if (_coord_type == Moose::COORD_XYZ)
  {
    _q_deta = &_fe[_tid]->get_dxyzdeta();
    _phi_deta = &_fe[_tid]->get_dphideta();
  }

  _fe[_tid]->reinit(_current_side_elem);

  if (_coord_type == Moose::COORD_XYZ)
  {
    if (_q_deta->empty())
      _q_deta = nullptr;
    if (_phi_deta->empty())
      _phi_deta = nullptr;
  }

  // Compute node map (given elem node, supply face node)
  _node_map.clear();
  const unsigned int num_node_elem = _current_elem->n_nodes();
  const Node * const * elem_nodes = _current_elem->get_nodes();
  const unsigned int num_node_face = _current_side_elem->n_nodes();
  const Node * const * face_nodes = _current_side_elem->get_nodes();
  unsigned int num_found = 0;
  for (unsigned i = 0; i < num_node_elem; ++i)
  {
    for (unsigned j = 0; j < num_node_face; ++j)
      if (**(elem_nodes + i) == **(face_nodes + j))
      {
        _node_map[i] = j;
        ++num_found;
        break;
      }
    if (num_found == num_node_face)
      break;
  }
}

void
PressureBase::precalculateQpOffDiagJacobian(const MooseVariableFEBase & /*jvar*/)
{
  precalculateQpJacobian();
}

template class PressureBaseTempl<false>;
template class PressureBaseTempl<true>;
