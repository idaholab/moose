//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADComputeIncrementalShellStrain.h"
#include "MooseMesh.h"
#include "Assembly.h"
#include "NonlinearSystem.h"
#include "MooseVariable.h"
#include "ArbitraryQuadrature.h"
#include "DenseMatrix.h"

#include "libmesh/quadrature.h"
#include "libmesh/utility.h"
#include "libmesh/enum_quadrature_type.h"
#include "libmesh/fe_type.h"
#include "libmesh/string_to_enum.h"
#include "libmesh/quadrature_gauss.h"

registerADMooseObject("TensorMechanicsApp", ADComputeIncrementalShellStrain);

defineADValidParams(
    ADComputeIncrementalShellStrain,
    ADMaterial,
    params.addClassDescription("Compute a small strain increment for the shell.");
    params.addRequiredCoupledVar(
        "rotations", "The rotations appropriate for the simulation geometry and coordinate system");
    params.addRequiredCoupledVar(
        "displacements",
        "The displacements appropriate for the simulation geometry and coordinate system");
    params.addRequiredCoupledVar(
        "thickness",
        "Cross-section area of the beam. Can be supplied as either a number or a variable name.");
    params.addRequiredParam<std::string>("order", "Quadrature order in out of plane direction");
    params.addParam<bool>("large_strain",
                          false,
                          "Set to true to turn on finite strain calculations."););

template <ComputeStage compute_stage>
ADComputeIncrementalShellStrain<compute_stage>::ADComputeIncrementalShellStrain(
    const InputParameters & parameters)
  : ADMaterial<compute_stage>(parameters),
    _nrot(coupledComponents("rotations")),
    _ndisp(coupledComponents("displacements")),
    _rot_num(_nrot),
    _disp_num(_ndisp),
    _thickness(coupledValue("thickness")),
    _large_strain(getParam<bool>("large_strain")),
    _strain_increment(),
    _total_strain(),
    _total_strain_old(),
    _nonlinear_sys(_fe_problem.getNonlinearSystemBase()),
    _soln_disp_index(4),
    _soln_rot_index(4),
    _soln_vector(20, 1),
    _strain_vector(5, 1),
    _nodes(4),
    _node_normal(declareADProperty<RealVectorValue>("node_normal")),
    _node_normal_old(getMaterialPropertyOldByName<RealVectorValue>("node_normal")),
    _dxyz_dxi(),
    _dxyz_deta(),
    _dxyz_dzeta(),
    _dxyz_dxi_old(),
    _dxyz_deta_old(),
    _dxyz_dzeta_old(),
    _V1(4),
    _V2(4),
    _B(),
    _B_old(),
    _ge(),
    _ge_old(),
    _Jmap(),
    _Jmap_old(),
    _sol(_nonlinear_sys.currentSolution()),
    _sol_old(_nonlinear_sys.solutionOld())
{
  // Checking for consistency between length of the provided displacements and rotations vector
  if (_ndisp != 3 || _nrot != 2)
    mooseError("ADComputeShellStrain: The number of variables supplied in 'displacements' "
               "must be 3 and that in 'rotations' must be 2.");

  // fetch coupled variables and gradients (as stateful properties if necessary)
  for (unsigned int i = 0; i < _ndisp; ++i)
  {
    MooseVariable * disp_variable = getVar("displacements", i);
    _disp_num[i] = disp_variable->number();

    if (i < _nrot)
    {
      MooseVariable * rot_variable = getVar("rotations", i);
      _rot_num[i] = rot_variable->number();
    }
  }

  _t_qrule = libmesh_make_unique<QGauss>(
      1, Utility::string_to_enum<Order>(getParam<std::string>("order")));
  _t_points = _t_qrule->get_points();
  _strain_increment.resize(_t_points.size());
  _total_strain.resize(_t_points.size());
  _total_strain_old.resize(_t_points.size());
  _B.resize(_t_points.size());
  _B_old.resize(_t_points.size());
  _ge.resize(_t_points.size());
  _ge_old.resize(_t_points.size());
  _Jmap.resize(_t_points.size());
  _Jmap_old.resize(_t_points.size());
  _dxyz_dxi.resize(_t_points.size());
  _dxyz_deta.resize(_t_points.size());
  _dxyz_dzeta.resize(_t_points.size());
  _dxyz_dxi_old.resize(_t_points.size());
  _dxyz_deta_old.resize(_t_points.size());
  _dxyz_dzeta_old.resize(_t_points.size());
  for (unsigned int i = 0; i < _t_points.size(); ++i)
  {
    _strain_increment[i] =
        &declareADProperty<RankTwoTensor>("strain_increment_t_points_" + std::to_string(i));
    _total_strain[i] =
        &declareADProperty<RankTwoTensor>("total_strain_t_points_" + std::to_string(i));
    _total_strain_old[i] =
        &getMaterialPropertyOldByName<RankTwoTensor>("total_strain_t_points_" + std::to_string(i));
    _B[i] = &declareADProperty<DenseMatrix<Real>>("B_matrix_t_points_" + std::to_string(i));
    _B_old[i] =
        &getMaterialPropertyOldByName<DenseMatrix<Real>>("B_matrix_t_points_" + std::to_string(i));
    _ge[i] = &declareADProperty<RankTwoTensor>("ge_matrix_t_points_" + std::to_string(i));
    _ge_old[i] =
        &getMaterialPropertyOldByName<RankTwoTensor>("ge_matrix_t_points_" + std::to_string(i));
    _Jmap[i] = &declareADProperty<Real>("J_mapping_t_points_" + std::to_string(i));
    _Jmap_old[i] = &getMaterialPropertyOldByName<Real>("J_mapping_t_points_" + std::to_string(i));
    _dxyz_dxi[i] = &declareADProperty<RealVectorValue>("dxyz_dxi_t_points_" + std::to_string(i));
    _dxyz_dxi_old[i] =
        &getMaterialPropertyOldByName<RealVectorValue>("dxyz_dxi_t_points_" + std::to_string(i));
    _dxyz_deta[i] = &declareADProperty<RealVectorValue>("dxyz_deta_t_points_" + std::to_string(i));
    _dxyz_deta_old[i] =
        &getMaterialPropertyOldByName<RealVectorValue>("dxyz_deta_t_points_" + std::to_string(i));
    _dxyz_dzeta[i] =
        &declareADProperty<RealVectorValue>("dxyz_dzeta_t_points_" + std::to_string(i));
    _dxyz_dzeta_old[i] =
        &getMaterialPropertyOldByName<RealVectorValue>("dxyz_dzeta_t_points_" + std::to_string(i));
  }
}

template <ComputeStage compute_stage>
void
ADComputeIncrementalShellStrain<compute_stage>::initQpStatefulProperties()
{
  computeGMatrix();
  computeBMatrix();
}

template <ComputeStage compute_stage>
void
ADComputeIncrementalShellStrain<compute_stage>::computeProperties()
{
  // quadrature points in isoparametric space
  _2d_points = _qrule->get_points(); // would be in 2D

  unsigned int dim = _current_elem->dim();
  if ((dim != 2))
    mooseError("Shell element is implemented only for 2D Linear elements");

  // derivatives of shape functions (dphidxi, dphideta and dphidzeta) evaluated at quadrature points
  // (in isoparametric space).
  FEType fe_type(Utility::string_to_enum<Order>("First"),
                 Utility::string_to_enum<FEFamily>("LAGRANGE"));
  auto & fe = _fe_problem.assembly(_tid).getFE(fe_type, dim);
  _dphidxi_map = fe->get_fe_map().get_dphidxi_map();
  _dphideta_map = fe->get_fe_map().get_dphideta_map();
  _phi_map = fe->get_fe_map().get_phi_map();

  for (unsigned int i = 0; i < 4; ++i)
    _nodes[i] = _current_elem->node_ptr(i);

  for (unsigned int i = 0; i < _2d_points.size(); ++i)
  {
    for (unsigned int j = 0; j < _t_points.size(); ++j)
    {
      (*_ge[j])[i] = (*_ge_old[j])[i];
      (*_Jmap[j])[i] = (*_Jmap_old[j])[i];
      (*_dxyz_dxi[j])[i] = (*_dxyz_dxi_old[j])[i];
      (*_dxyz_deta[j])[i] = (*_dxyz_deta_old[j])[i];
      (*_dxyz_dzeta[j])[i] = (*_dxyz_dzeta_old[j])[i];
      (*_B[j])[i] = (*_B_old[j])[i];
    }
  }

  computeSolnVector();

  computeNodeNormal();

  for (unsigned int i = 0; i < _2d_points.size(); ++i)
  {
    for (unsigned int j = 0; j < _t_points.size(); ++j)
    {
      // compute strain increment in covariant coordinate system using B and _soln_vector
      for (unsigned int temp1 = 0; temp1 < 5; ++temp1)
      {
        _strain_vector(temp1) = 0.0;
        for (unsigned int temp2 = 0; temp2 < 20; ++temp2)
          _strain_vector(temp1) += (*_B[j])[i](temp1, temp2) * _soln_vector(temp2);
      }

      (*_strain_increment[j])[i](0, 0) = _strain_vector(0);
      (*_strain_increment[j])[i](1, 1) = _strain_vector(1);
      (*_strain_increment[j])[i](0, 1) = _strain_vector(2);
      (*_strain_increment[j])[i](0, 2) = _strain_vector(3);
      (*_strain_increment[j])[i](1, 2) = _strain_vector(4);
      (*_strain_increment[j])[i](1, 0) = (*_strain_increment[j])[i](0, 1);
      (*_strain_increment[j])[i](2, 0) = (*_strain_increment[j])[i](0, 2);
      (*_strain_increment[j])[i](2, 1) = (*_strain_increment[j])[i](1, 2);
      (*_total_strain[j])[i] = (*_total_strain_old[j])[i] + (*_strain_increment[j])[i];
    }
  }
  copyDualNumbersToValues();
}

template <ComputeStage compute_stage>
void
ADComputeIncrementalShellStrain<compute_stage>::computeGMatrix()
{
  // quadrature points in isoparametric space
  _2d_points = _qrule->get_points(); // would be in 2D

  unsigned int dim = _current_elem->dim();
  if ((dim != 2))
    mooseError("Shell element is implemented only for 2D Linear elements");

  // derivatives of shape functions (dphidxi, dphideta and dphidzeta) evaluated at quadrature points
  // (in isoparametric space).
  FEType fe_type(Utility::string_to_enum<Order>("First"),
                 Utility::string_to_enum<FEFamily>("LAGRANGE"));
  auto & fe = _fe_problem.assembly(_tid).getFE(fe_type, dim);
  _dphidxi_map = fe->get_fe_map().get_dphidxi_map();
  _dphideta_map = fe->get_fe_map().get_dphideta_map();
  _phi_map = fe->get_fe_map().get_phi_map();

  for (unsigned int i = 0; i < 4; ++i)
    _nodes[i] = _current_elem->node_ptr(i);

  ADRealVectorValue x = (*_nodes[1] - *_nodes[0]);
  ADRealVectorValue y = (*_nodes[3] - *_nodes[0]);
  ADRealVectorValue normal = x.cross(y);
  normal /= normal.norm();

  for (unsigned int k = 0; k < 4; ++k)
    _node_normal[k] = normal;

  ADRankTwoTensor a;
  ADDenseMatrix b(5, 20);
  ADRealVectorValue c;
  for (unsigned int t = 0; t < _t_points.size(); ++t)
  {
    (*_strain_increment[t])[_qp] = a;
    (*_total_strain[t])[_qp] = a;
    (*_B[t])[_qp] = b;
    (*_ge[t])[_qp] = a;
    (*_Jmap[t])[_qp] = 0;
    (*_dxyz_dxi[t])[_qp] = c;
    (*_dxyz_deta[t])[_qp] = c;
    (*_dxyz_dzeta[t])[_qp] = c;
  }

  // calculating derivatives of shape function is physical space (dphi/dx, dphi/dy, dphi/dz) at
  // quadrature points these are g_{i} in Dvorkin's paper
  ADRealVectorValue en;
  ADRealVectorValue ex;
  ADRealVectorValue ey;
  en(2) = 1.0;
  ex(0) = 1.0;
  ey(1) = 1.0;
  for (unsigned int i = 0; i < _2d_points.size(); ++i)
  {
    for (unsigned int j = 0; j < _t_points.size(); ++j)
    {
      (*_dxyz_dxi[j])[i].zero();
      for (unsigned int component = 0; component < 3; ++component)
      {
        (*_dxyz_dxi[j])[i](component) = 0.0;
        (*_dxyz_deta[j])[i](component) = 0.0;
        (*_dxyz_dzeta[j])[i](component) = 0.0;

        for (unsigned int k = 0; k < _nodes.size(); ++k)
        {
          (*_dxyz_dxi[j])[i](component) +=
              _dphidxi_map[k][i] * ((*_nodes[k])(component)) +
              _t_points[j](0) / 2.0 * _thickness[i] * _dphidxi_map[k][i] * en(component);
          (*_dxyz_deta[j])[i](component) +=
              _dphideta_map[k][i] * ((*_nodes[k])(component)) +
              _t_points[j](0) / 2.0 * _thickness[i] * _dphideta_map[k][i] * en(component);
          (*_dxyz_dzeta[j])[i](component) += _thickness[i] * _phi_map[k][i] * en(component) / 2.0;
        }
      }
    }
  }

  for (unsigned int i = 0; i < _2d_points.size(); ++i)
  {
    for (unsigned int j = 0; j < _t_points.size(); ++j)
    {
      // calculate gij for elasticity tensor
      ADRankTwoTensor gmn;
      for (unsigned int component = 0; component < 3; ++component)
      {
        gmn(0, 0) += (*_dxyz_dxi[j])[i](component) * (*_dxyz_dxi[j])[i](component);
        gmn(1, 1) += (*_dxyz_deta[j])[i](component) * (*_dxyz_deta[j])[i](component);
        gmn(2, 2) += (*_dxyz_dzeta[j])[i](component) * (*_dxyz_dzeta[j])[i](component);
        gmn(0, 1) += (*_dxyz_dxi[j])[i](component) * (*_dxyz_deta[j])[i](component);
        gmn(0, 2) += (*_dxyz_dxi[j])[i](component) * (*_dxyz_dzeta[j])[i](component);
        gmn(1, 2) += (*_dxyz_deta[j])[i](component) * (*_dxyz_dzeta[j])[i](component);
      }
      gmn(1, 0) = gmn(0, 1);
      gmn(2, 0) = gmn(0, 2);
      gmn(2, 1) = gmn(1, 2);

      ADRankTwoTensor gmninv = gmn.inverse();
      (*_Jmap[j])[i] = std::sqrt(gmn.det());

      // calculate ge
      ADRealVectorValue e3 = (*_dxyz_dzeta[j])[i] / (*_dxyz_dzeta[j])[i].norm();
      ADRealVectorValue e1 = (*_dxyz_deta[j])[i].cross(e3);
      e1 /= e1.norm();
      ADRealVectorValue e2 = e3.cross(e1);
      e2 /= e2.norm();

      (*_ge[j])[i](0, 0) = (gmninv * (*_dxyz_dxi[j])[i]) * e1;
      (*_ge[j])[i](0, 1) = (gmninv * (*_dxyz_dxi[j])[i]) * e2;
      (*_ge[j])[i](0, 2) = (gmninv * (*_dxyz_dxi[j])[i]) * e3;
      (*_ge[j])[i](1, 0) = (gmninv * (*_dxyz_deta[j])[i]) * e1;
      (*_ge[j])[i](1, 1) = (gmninv * (*_dxyz_deta[j])[i]) * e2;
      (*_ge[j])[i](1, 2) = (gmninv * (*_dxyz_deta[j])[i]) * e3;
      (*_ge[j])[i](2, 0) = (gmninv * (*_dxyz_dzeta[j])[i]) * e1;
      (*_ge[j])[i](2, 1) = (gmninv * (*_dxyz_dzeta[j])[i]) * e2;
      (*_ge[j])[i](2, 2) = (gmninv * (*_dxyz_dzeta[j])[i]) * e3;
    }
  }
}

template <ComputeStage compute_stage>
void
ADComputeIncrementalShellStrain<compute_stage>::computeNodeNormal()
{
  for (unsigned int k = 0; k < _nodes.size(); ++k)
    _node_normal[k] = _node_normal_old[k];
}

template <ComputeStage compute_stage>
void
ADComputeIncrementalShellStrain<compute_stage>::computeBMatrix()
{
  // compute nodal local axis
  _x2(1) = 1;
  _x3(2) = 1;

  for (unsigned int k = 0; k < _nodes.size(); ++k)
  {
    _V1[k] = _x2.cross(_node_normal[k]);
    _V1[k] /= _x2.norm() * _node_normal[k].norm();

    // If x2 is parallel to node normal, set V1 to x3
    if (MooseUtils::absoluteFuzzyEqual(_V1[k].norm(), 0.0, 1e-6))
      _V1[k] = _x3;

    _V2[k] = _node_normal[k].cross(_V1[k]);
  }

  // compute B matrix rows correspond to [ux1, ux2, ux3, ux4, uy1, uy2, uy3, uy4, uz1, uz2, uz3,
  // uz4, a1, a2, a3, a4, b1, b2, b3, b4]
  for (unsigned int i = 0; i < _2d_points.size(); ++i)
  {
    for (unsigned int j = 0; j < _t_points.size(); ++j)
    {
      (*_B[j])[i].resize(5, 20);
      (*_B[j])[i].zero();
      for (unsigned int k = 0; k < _nodes.size(); ++k)
      {
        // corresponding to strain(0,0)
        (*_B[j])[i](0, k) += _dphidxi_map[k][i] * (*_dxyz_dxi[j])[i](0);
        (*_B[j])[i](0, 4 + k) = _dphidxi_map[k][i] * (*_dxyz_dxi[j])[i](1);
        (*_B[j])[i](0, 8 + k) = _dphidxi_map[k][i] * (*_dxyz_dxi[j])[i](2);
        (*_B[j])[i](0, 12 + k) = _dphidxi_map[k][i] * _t_points[j](0) / 2.0 * _thickness[i] *
                                 (-_V2[k] * (*_dxyz_dxi[j])[i]);
        (*_B[j])[i](0, 16 + k) = _dphidxi_map[k][i] * _t_points[j](0) / 2.0 * _thickness[i] *
                                 (_V1[k] * (*_dxyz_dxi[j])[i]);

        // corresponding to strain(1,1)
        (*_B[j])[i](1, k) = _dphideta_map[k][i] * (*_dxyz_deta[j])[i](0);
        (*_B[j])[i](1, 4 + k) = _dphideta_map[k][i] * (*_dxyz_deta[j])[i](1);
        (*_B[j])[i](1, 8 + k) = _dphideta_map[k][i] * (*_dxyz_deta[j])[i](2);
        (*_B[j])[i](1, 12 + k) = _dphideta_map[k][i] * _t_points[j](0) / 2.0 * _thickness[i] *
                                 (-_V2[k] * (*_dxyz_deta[j])[i]);
        (*_B[j])[i](1, 16 + k) = _dphideta_map[k][i] * _t_points[j](0) / 2.0 * _thickness[i] *
                                 (_V1[k] * (*_dxyz_deta[j])[i]);

        // corresponding to strain(2,2) = 0

        // corresponding to strain(0,1)
        (*_B[j])[i](2, k) = 0.5 * (_dphideta_map[k][i] * (*_dxyz_dxi[j])[i](0) +
                                   _dphidxi_map[k][i] * (*_dxyz_deta[j])[i](0));
        (*_B[j])[i](2, 4 + k) = 0.5 * (_dphideta_map[k][i] * (*_dxyz_dxi[j])[i](1) +
                                       _dphidxi_map[k][i] * (*_dxyz_deta[j])[i](1));
        (*_B[j])[i](2, 8 + k) = 0.5 * (_dphideta_map[k][i] * (*_dxyz_dxi[j])[i](2) +
                                       _dphidxi_map[k][i] * (*_dxyz_deta[j])[i](2));
        (*_B[j])[i](2, 12 + k) =
            0.25 * _t_points[j](0) * _thickness[i] * -_V2[k] *
            (_dphideta_map[k][i] * (*_dxyz_dxi[j])[i] + _dphidxi_map[k][i] * (*_dxyz_deta[j])[i]);
        (*_B[j])[i](2, 16 + k) =
            0.25 * _t_points[j](0) * _thickness[i] * _V1[k] *
            ((*_dxyz_deta[j])[i] * _dphidxi_map[k][i] + (*_dxyz_dxi[j])[i] * _dphideta_map[k][i]);
      }

      _g3_A = _thickness[i] / 4.0 * (_node_normal[2] + _node_normal[3]);
      _g3_C = _thickness[i] / 4.0 * (_node_normal[0] + _node_normal[1]);
      _g3_B = _thickness[i] / 4.0 * (_node_normal[0] + _node_normal[3]);
      _g3_D = _thickness[i] / 4.0 * (_node_normal[1] + _node_normal[2]);

      _g1_A = 0.5 * ((*_nodes[2]) - (*_nodes[3])) +
              _t_points[j](0) / 4.0 * _thickness[i] * (_node_normal[2] - _node_normal[3]);
      _g1_C = 0.5 * ((*_nodes[1]) - (*_nodes[0])) +
              _t_points[j](0) / 4.0 * _thickness[i] * (_node_normal[1] - _node_normal[0]);
      _g2_B = 0.5 * ((*_nodes[3]) - (*_nodes[0])) +
              _t_points[j](0) / 4.0 * _thickness[i] * (_node_normal[3] - _node_normal[0]);
      _g2_D = 0.5 * ((*_nodes[2]) - (*_nodes[1])) +
              _t_points[j](0) / 4.0 * _thickness[i] * (_node_normal[2] - _node_normal[1]);

      updateGVectors(); // for large strain problems

      // corresponding to strain(0,2)
      for (unsigned int component = 0; component < 3; component++)
      {
        (*_B[j])[i](3, 2 + component * 4) = 1.0 / 8.0 * (1.0 + _2d_points[i](1)) * _g3_A(component);
        (*_B[j])[i](3, 3 + component * 4) =
            1.0 / 8.0 * (1.0 + _2d_points[i](1)) * -_g3_A(component);
        (*_B[j])[i](3, 1 + component * 4) = 1.0 / 8.0 * (1.0 - _2d_points[i](1)) * _g3_C(component);
        (*_B[j])[i](3, component * 4) = 1.0 / 8.0 * (1.0 - _2d_points[i](1)) * -_g3_C(component);
      }
      (*_B[j])[i](3, 14) =
          1.0 / 8.0 * (1.0 + _2d_points[i](1)) * 0.5 * _thickness[i] * _g1_A * -_V2[2];
      (*_B[j])[i](3, 18) =
          1.0 / 8.0 * (1.0 + _2d_points[i](1)) * 0.5 * _thickness[i] * _g1_A * _V1[2];
      (*_B[j])[i](3, 15) =
          1.0 / 8.0 * (1.0 + _2d_points[i](1)) * 0.5 * _thickness[i] * _g1_A * -_V2[3];
      (*_B[j])[i](3, 19) =
          1.0 / 8.0 * (1.0 + _2d_points[i](1)) * 0.5 * _thickness[i] * _g1_A * _V1[3];

      (*_B[j])[i](3, 13) =
          1.0 / 8.0 * (1.0 - _2d_points[i](1)) * 0.5 * _thickness[i] * _g1_C * -_V2[1];
      (*_B[j])[i](3, 17) =
          1.0 / 8.0 * (1.0 - _2d_points[i](1)) * 0.5 * _thickness[i] * _g1_C * _V1[1];
      (*_B[j])[i](3, 12) =
          1.0 / 8.0 * (1.0 - _2d_points[i](1)) * 0.5 * _thickness[i] * _g1_C * -_V2[0];
      (*_B[j])[i](3, 16) =
          1.0 / 8.0 * (1.0 - _2d_points[i](1)) * 0.5 * _thickness[i] * _g1_C * _V1[0];

      // corresponding to strain(1,2)
      for (unsigned int component = 0; component < 3; component++)
      {
        (*_B[j])[i](4, 2 + component * 4) = 1.0 / 8.0 * (1.0 + _2d_points[i](0)) * _g3_D(component);
        (*_B[j])[i](4, 1 + component * 4) =
            1.0 / 8.0 * (1.0 + _2d_points[i](0)) * -_g3_D(component);
        (*_B[j])[i](4, 3 + component * 4) = 1.0 / 8.0 * (1.0 - _2d_points[i](0)) * _g3_B(component);
        (*_B[j])[i](4, component * 4) = 1.0 / 8.0 * (1.0 - _2d_points[i](0)) * -_g3_B(component);
      }
      (*_B[j])[i](4, 14) =
          1.0 / 8.0 * (1.0 + _2d_points[i](0)) * 0.5 * _thickness[i] * _g2_D * -_V2[2];
      (*_B[j])[i](4, 18) =
          1.0 / 8.0 * (1.0 + _2d_points[i](0)) * 0.5 * _thickness[i] * _g2_D * _V1[2];
      (*_B[j])[i](4, 13) =
          1.0 / 8.0 * (1.0 + _2d_points[i](0)) * 0.5 * _thickness[i] * _g2_D * -_V2[1];
      (*_B[j])[i](4, 17) =
          1.0 / 8.0 * (1.0 + _2d_points[i](0)) * 0.5 * _thickness[i] * _g2_D * _V1[1];

      (*_B[j])[i](4, 15) =
          1.0 / 8.0 * (1.0 - _2d_points[i](0)) * 0.5 * _thickness[i] * _g2_B * -_V2[3];
      (*_B[j])[i](4, 19) =
          1.0 / 8.0 * (1.0 - _2d_points[i](0)) * 0.5 * _thickness[i] * _g2_B * _V1[3];
      (*_B[j])[i](4, 12) =
          1.0 / 8.0 * (1.0 - _2d_points[i](0)) * 0.5 * _thickness[i] * _g2_B * -_V2[0];
      (*_B[j])[i](4, 16) =
          1.0 / 8.0 * (1.0 - _2d_points[i](0)) * 0.5 * _thickness[i] * _g2_B * _V1[0];
    }
  }
}

template <>
void
ADComputeIncrementalShellStrain<RESIDUAL>::computeSolnVector()
{
  _soln_vector.zero();

  for (unsigned int j = 0; j < 4; ++j)
  {
    _soln_disp_index[j].resize(_ndisp);
    _soln_rot_index[j].resize(_nrot);

    for (unsigned int i = 0; i < _ndisp; ++i)
    {
      _soln_disp_index[j][i] = _nodes[j]->dof_number(_nonlinear_sys.number(), _disp_num[i], 0);
      _soln_vector(j + i * _nodes.size()) =
          (*_sol)(_soln_disp_index[j][i]) - _sol_old(_soln_disp_index[j][i]);
    }

    for (unsigned int i = 0; i < _nrot; ++i)
    {
      _soln_rot_index[j][i] = _nodes[j]->dof_number(_nonlinear_sys.number(), _rot_num[i], 0);
      _soln_vector(j + 12 + i * _nodes.size()) =
          (*_sol)(_soln_rot_index[j][i]) - _sol_old(_soln_rot_index[j][i]);
    }
  }
}

template <>
void
ADComputeIncrementalShellStrain<JACOBIAN>::computeSolnVector()
{
  _soln_vector.zero();

  for (unsigned int j = 0; j < 4; ++j)
  {
    _soln_disp_index[j].resize(_ndisp);
    _soln_rot_index[j].resize(_nrot);

    for (unsigned int i = 0; i < _ndisp; ++i)
    {
      size_t ad_offset = _disp_num[i] * _nonlinear_sys.getMaxVarNDofsPerElem();
      _soln_disp_index[j][i] = _nodes[j]->dof_number(_nonlinear_sys.number(), _disp_num[i], 0);
      _soln_vector(j + i * _nodes.size()) =
          (*_sol)(_soln_disp_index[j][i]) - _sol_old(_soln_disp_index[j][i]);
      Moose::derivInsert(_soln_vector(j + i * _nodes.size()).derivatives(), ad_offset + j, 1.);
    }

    for (unsigned int i = 0; i < _nrot; ++i)
    {
      size_t ad_offset = _rot_num[i] * _nonlinear_sys.getMaxVarNDofsPerElem();
      _soln_rot_index[j][i] = _nodes[j]->dof_number(_nonlinear_sys.number(), _rot_num[i], 0);
      _soln_vector(j + 12 + i * _nodes.size()) =
          (*_sol)(_soln_rot_index[j][i]) - _sol_old(_soln_rot_index[j][i]);
      Moose::derivInsert(_soln_vector(j + 12 + i * _nodes.size()).derivatives(), ad_offset + j, 1.);
    }
  }
}
