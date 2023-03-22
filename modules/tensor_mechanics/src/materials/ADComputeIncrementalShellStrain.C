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

registerMooseObject("TensorMechanicsApp", ADComputeIncrementalShellStrain);

InputParameters
ADComputeIncrementalShellStrain::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Compute a small strain increment for the shell.");
  params.addRequiredCoupledVar(
      "rotations", "The rotations appropriate for the simulation geometry and coordinate system");
  params.addRequiredCoupledVar(
      "displacements",
      "The displacements appropriate for the simulation geometry and coordinate system");
  params.addRequiredCoupledVar(
      "thickness",
      "Thickness of the shell. Can be supplied as either a number or a variable name.");
  params.addRequiredParam<std::string>("through_thickness_order",
                                       "Quadrature order in out of plane direction");
  params.addParam<bool>(
      "large_strain", false, "Set to true to turn on finite strain calculations.");
  return params;
}

ADComputeIncrementalShellStrain::ADComputeIncrementalShellStrain(const InputParameters & parameters)
  : Material(parameters),
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
    _v1(4),
    _v2(4),
    _B(),
    _B_old(),
    _ge(),
    _ge_old(),
    _J_map(),
    _J_map_old(),
    _covariant_transformation_matrix(),
    _covariant_transformation_matrix_old(),
    _contravariant_transformation_matrix(),
    _contravariant_transformation_matrix_old(),
    _total_global_strain(),
    _sol(_nonlinear_sys.currentSolution()),
    _sol_old(_nonlinear_sys.solutionOld())
{
  // Checking for consistency between length of the provided displacements and rotations vector
  if (_ndisp != 3 || _nrot != 2)
    mooseError(
        "ADComputeIncrementalShellStrain: The number of variables supplied in 'displacements' "
        "must be 3 and that in 'rotations' must be 2.");

  if (_mesh.hasSecondOrderElements())
    mooseError(
        "ADComputeIncrementalShellStrain: Shell element is implemented only for linear elements.");

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

  _t_qrule = std::make_unique<QGauss>(
      1, Utility::string_to_enum<Order>(getParam<std::string>("through_thickness_order")));
  _t_points = _t_qrule->get_points();
  _strain_increment.resize(_t_points.size());
  _total_strain.resize(_t_points.size());
  _total_strain_old.resize(_t_points.size());
  _B.resize(_t_points.size());
  _B_old.resize(_t_points.size());
  _ge.resize(_t_points.size());
  _ge_old.resize(_t_points.size());
  _J_map.resize(_t_points.size());
  _J_map_old.resize(_t_points.size());
  _dxyz_dxi.resize(_t_points.size());
  _dxyz_deta.resize(_t_points.size());
  _dxyz_dzeta.resize(_t_points.size());
  _dxyz_dxi_old.resize(_t_points.size());
  _dxyz_deta_old.resize(_t_points.size());
  _dxyz_dzeta_old.resize(_t_points.size());
  _covariant_transformation_matrix.resize(_t_points.size());
  _covariant_transformation_matrix_old.resize(_t_points.size());
  _contravariant_transformation_matrix.resize(_t_points.size());
  _contravariant_transformation_matrix_old.resize(_t_points.size());
  _total_global_strain.resize(_t_points.size());

  _transformation_matrix = &declareADProperty<RankTwoTensor>("transformation_matrix_element");

  for (unsigned int i = 0; i < _t_points.size(); ++i)
  {
    _strain_increment[i] =
        &declareADProperty<RankTwoTensor>("strain_increment_t_points_" + std::to_string(i));
    _total_strain[i] =
        &declareADProperty<RankTwoTensor>("total_strain_t_points_" + std::to_string(i));
    _total_strain_old[i] =
        &getMaterialPropertyOldByName<RankTwoTensor>("total_strain_t_points_" + std::to_string(i));
    _B[i] = &declareADProperty<DenseMatrix<Real>>("B_t_points_" + std::to_string(i));
    _B_old[i] = &getMaterialPropertyOldByName<DenseMatrix<Real>>("B_t_points_" + std::to_string(i));
    _ge[i] = &declareADProperty<RankTwoTensor>("ge_t_points_" + std::to_string(i));
    _ge_old[i] = &getMaterialPropertyOldByName<RankTwoTensor>("ge_t_points_" + std::to_string(i));
    _J_map[i] = &declareADProperty<Real>("J_mapping_t_points_" + std::to_string(i));
    _J_map_old[i] = &getMaterialPropertyOldByName<Real>("J_mapping_t_points_" + std::to_string(i));
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
    // Create rotation matrix and total strain global for output purposes only
    _covariant_transformation_matrix[i] =
        &declareProperty<RankTwoTensor>("covariant_transformation_t_points_" + std::to_string(i));
    _covariant_transformation_matrix_old[i] = &getMaterialPropertyOldByName<RankTwoTensor>(
        "covariant_transformation_t_points_" + std::to_string(i));
    _contravariant_transformation_matrix[i] = &declareProperty<RankTwoTensor>(
        "contravariant_transformation_t_points_" + std::to_string(i));
    _contravariant_transformation_matrix_old[i] = &getMaterialPropertyOldByName<RankTwoTensor>(
        "contravariant_transformation_t_points_" + std::to_string(i));
    _total_global_strain[i] =
        &declareProperty<RankTwoTensor>("total_global_strain_t_points_" + std::to_string(i));
  }

  // used later for computing local coordinate system
  _x2(1) = 1;
  _x3(2) = 1;
}

void
ADComputeIncrementalShellStrain::initQpStatefulProperties()
{
  unsigned int dim = _current_elem->dim();
  if ((dim != 2))
    mooseError(
        "ADComputeIncrementalShellStrain: Shell element is implemented only for 2D elements");
  if (_current_elem->n_nodes() != 4)
    mooseError("ADComputeIncrementalShellStrain: Shell element needs to have exactly four nodes.");
  if (_qrule->get_points().size() != 4)
    mooseError("ADComputeIncrementalShellStrain: Shell element needs to have exactly four "
               "quadrature points.");
  computeGMatrix();
  computeBMatrix();
}

void
ADComputeIncrementalShellStrain::computeProperties()
{
  // quadrature points in isoparametric space
  _2d_points = _qrule->get_points(); // would be in 2D

  // derivatives of shape functions (dphidxi, dphideta and dphidzeta) evaluated at quadrature points
  // (in isoparametric space).
  unsigned int dim = _current_elem->dim();
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
      (*_J_map[j])[i] = (*_J_map_old[j])[i];
      (*_dxyz_dxi[j])[i] = (*_dxyz_dxi_old[j])[i];
      (*_dxyz_deta[j])[i] = (*_dxyz_deta_old[j])[i];
      (*_dxyz_dzeta[j])[i] = (*_dxyz_dzeta_old[j])[i];
      (*_B[j])[i] = (*_B_old[j])[i];
      (*_covariant_transformation_matrix[j])[i] = (*_covariant_transformation_matrix_old[j])[i];
      (*_contravariant_transformation_matrix[j])[i] =
          (*_contravariant_transformation_matrix_old[j])[i];
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

      for (unsigned int ii = 0; ii < 3; ++ii)
        for (unsigned int jj = 0; jj < 3; ++jj)
          _unrotated_total_strain(ii, jj) = MetaPhysicL::raw_value((*_total_strain[j])[i](ii, jj));
      (*_total_global_strain[j])[i] = (*_contravariant_transformation_matrix[j])[i] *
                                      _unrotated_total_strain *
                                      (*_contravariant_transformation_matrix[j])[i].transpose();
    }
  }
}

void
ADComputeIncrementalShellStrain::computeGMatrix()
{
  // quadrature points in isoparametric space
  _2d_points = _qrule->get_points(); // would be in 2D

  unsigned int dim = _current_elem->dim();

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
  RankTwoTensor d;
  for (unsigned int t = 0; t < _t_points.size(); ++t)
  {
    (*_strain_increment[t])[_qp] = a;
    (*_total_strain[t])[_qp] = a;
    (*_B[t])[_qp] = b;
    (*_ge[t])[_qp] = a;
    (*_J_map[t])[_qp] = 0;
    (*_dxyz_dxi[t])[_qp] = c;
    (*_dxyz_deta[t])[_qp] = c;
    (*_dxyz_dzeta[t])[_qp] = c;
    (*_covariant_transformation_matrix[t])[_qp] = d;
    (*_contravariant_transformation_matrix[t])[_qp] = d;
  }

  // calculating derivatives of shape function in physical space (dphi/dx, dphi/dy, dphi/dz) at
  // quadrature points these are g_{i} in Dvorkin's paper
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
          (*_dxyz_dxi[j])[i](component) += _dphidxi_map[k][i] * ((*_nodes[k])(component)) +
                                           _t_points[j](0) / 2.0 * _thickness[i] *
                                               _dphidxi_map[k][i] * _node_normal[k](component);
          (*_dxyz_deta[j])[i](component) += _dphideta_map[k][i] * ((*_nodes[k])(component)) +
                                            _t_points[j](0) / 2.0 * _thickness[i] *
                                                _dphideta_map[k][i] * _node_normal[k](component);
          (*_dxyz_dzeta[j])[i](component) +=
              _thickness[i] * _phi_map[k][i] * _node_normal[k](component) / 2.0;
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
      RankTwoTensor J;
      for (unsigned int component = 0; component < 3; ++component)
      {
        gmn(0, 0) += (*_dxyz_dxi[j])[i](component) * (*_dxyz_dxi[j])[i](component);
        gmn(1, 1) += (*_dxyz_deta[j])[i](component) * (*_dxyz_deta[j])[i](component);
        gmn(2, 2) += (*_dxyz_dzeta[j])[i](component) * (*_dxyz_dzeta[j])[i](component);
        gmn(0, 1) += (*_dxyz_dxi[j])[i](component) * (*_dxyz_deta[j])[i](component);
        gmn(0, 2) += (*_dxyz_dxi[j])[i](component) * (*_dxyz_dzeta[j])[i](component);
        gmn(1, 2) += (*_dxyz_deta[j])[i](component) * (*_dxyz_dzeta[j])[i](component);

        J(0, component) = MetaPhysicL::raw_value((*_dxyz_dxi[j])[i](component));
        J(1, component) = MetaPhysicL::raw_value((*_dxyz_deta[j])[i](component));
        J(2, component) = MetaPhysicL::raw_value((*_dxyz_dzeta[j])[i](component));
      }
      gmn(1, 0) = gmn(0, 1);
      gmn(2, 0) = gmn(0, 2);
      gmn(2, 1) = gmn(1, 2);

      ADRankTwoTensor gmninv_temp = gmn.inverse();
      (*_J_map[j])[i] = std::sqrt(gmn.det());
      (*_covariant_transformation_matrix[j])[i] = J;

      (*_contravariant_transformation_matrix[j])[i] =
          (*_covariant_transformation_matrix[j])[i].inverse();

      Real normx = std::sqrt(J(0, 0) * J(0, 0) + J(0, 1) * J(0, 1) + J(0, 2) * J(0, 2));
      Real normy = std::sqrt(J(1, 0) * J(1, 0) + J(1, 1) * J(1, 1) + J(1, 2) * J(1, 2));
      Real normz = std::sqrt(J(2, 0) * J(2, 0) + J(2, 1) * J(2, 1) + J(2, 2) * J(2, 2));

      J(0, 0) /= normx;
      J(0, 1) /= normx;
      J(0, 2) /= normx;

      J(1, 0) /= normy;
      J(1, 1) /= normy;
      J(1, 2) /= normy;

      J(2, 0) /= normz;
      J(2, 1) /= normz;
      J(2, 2) /= normz;

      (*_transformation_matrix)[i] = J;

      // calculate ge
      ADRealVectorValue e3 = (*_dxyz_dzeta[j])[i] / (*_dxyz_dzeta[j])[i].norm();
      ADRealVectorValue e1 = (*_dxyz_deta[j])[i].cross(e3);
      e1 /= e1.norm();
      ADRealVectorValue e2 = e3.cross(e1);
      e2 /= e2.norm();

      ADRankTwoTensor local_rotation_mat;
      local_rotation_mat(0, 0) = e1(0);
      local_rotation_mat(0, 1) = e1(1);
      local_rotation_mat(0, 2) = e1(2);
      local_rotation_mat(1, 0) = e2(0);
      local_rotation_mat(1, 1) = e2(1);
      local_rotation_mat(1, 2) = e2(2);
      local_rotation_mat(2, 0) = e3(0);
      local_rotation_mat(2, 1) = e3(1);
      local_rotation_mat(2, 2) = e3(2);

      ADRankTwoTensor gmninv = local_rotation_mat.transpose() * gmninv_temp * local_rotation_mat;

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

void
ADComputeIncrementalShellStrain::computeNodeNormal()
{
  for (unsigned int k = 0; k < _nodes.size(); ++k)
    _node_normal[k] = _node_normal_old[k];
}

void
ADComputeIncrementalShellStrain::computeBMatrix()
{
  // compute nodal local axis
  for (unsigned int k = 0; k < _nodes.size(); ++k)
  {
    _v1[k] = _x2.cross(_node_normal[k]);
    _v1[k] /= _x2.norm() * _node_normal[k].norm();

    // If x2 is parallel to node normal, set V1 to x3
    if (MooseUtils::absoluteFuzzyEqual(_v1[k].norm(), 0.0, 1e-6))
      _v1[k] = _x3;

    _v2[k] = _node_normal[k].cross(_v1[k]);
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
                                 (-_v2[k] * (*_dxyz_dxi[j])[i]);
        (*_B[j])[i](0, 16 + k) = _dphidxi_map[k][i] * _t_points[j](0) / 2.0 * _thickness[i] *
                                 (_v1[k] * (*_dxyz_dxi[j])[i]);

        // corresponding to strain(1,1)
        (*_B[j])[i](1, k) = _dphideta_map[k][i] * (*_dxyz_deta[j])[i](0);
        (*_B[j])[i](1, 4 + k) = _dphideta_map[k][i] * (*_dxyz_deta[j])[i](1);
        (*_B[j])[i](1, 8 + k) = _dphideta_map[k][i] * (*_dxyz_deta[j])[i](2);
        (*_B[j])[i](1, 12 + k) = _dphideta_map[k][i] * _t_points[j](0) / 2.0 * _thickness[i] *
                                 (-_v2[k] * (*_dxyz_deta[j])[i]);
        (*_B[j])[i](1, 16 + k) = _dphideta_map[k][i] * _t_points[j](0) / 2.0 * _thickness[i] *
                                 (_v1[k] * (*_dxyz_deta[j])[i]);

        // corresponding to strain(2,2) = 0

        // corresponding to strain(0,1)
        (*_B[j])[i](2, k) = 0.5 * (_dphideta_map[k][i] * (*_dxyz_dxi[j])[i](0) +
                                   _dphidxi_map[k][i] * (*_dxyz_deta[j])[i](0));
        (*_B[j])[i](2, 4 + k) = 0.5 * (_dphideta_map[k][i] * (*_dxyz_dxi[j])[i](1) +
                                       _dphidxi_map[k][i] * (*_dxyz_deta[j])[i](1));
        (*_B[j])[i](2, 8 + k) = 0.5 * (_dphideta_map[k][i] * (*_dxyz_dxi[j])[i](2) +
                                       _dphidxi_map[k][i] * (*_dxyz_deta[j])[i](2));
        (*_B[j])[i](2, 12 + k) =
            0.25 * _t_points[j](0) * _thickness[i] * -_v2[k] *
            (_dphideta_map[k][i] * (*_dxyz_dxi[j])[i] + _dphidxi_map[k][i] * (*_dxyz_deta[j])[i]);
        (*_B[j])[i](2, 16 + k) =
            0.25 * _t_points[j](0) * _thickness[i] * _v1[k] *
            ((*_dxyz_deta[j])[i] * _dphidxi_map[k][i] + (*_dxyz_dxi[j])[i] * _dphideta_map[k][i]);
      }

      _g3_a = _thickness[i] / 4.0 * (_node_normal[2] + _node_normal[3]);
      _g3_c = _thickness[i] / 4.0 * (_node_normal[0] + _node_normal[1]);
      _g3_b = _thickness[i] / 4.0 * (_node_normal[0] + _node_normal[3]);
      _g3_d = _thickness[i] / 4.0 * (_node_normal[1] + _node_normal[2]);

      _g1_a = 0.5 * ((*_nodes[2]) - (*_nodes[3])) +
              _t_points[j](0) / 4.0 * _thickness[i] * (_node_normal[2] - _node_normal[3]);
      _g1_c = 0.5 * ((*_nodes[1]) - (*_nodes[0])) +
              _t_points[j](0) / 4.0 * _thickness[i] * (_node_normal[1] - _node_normal[0]);
      _g2_b = 0.5 * ((*_nodes[3]) - (*_nodes[0])) +
              _t_points[j](0) / 4.0 * _thickness[i] * (_node_normal[3] - _node_normal[0]);
      _g2_d = 0.5 * ((*_nodes[2]) - (*_nodes[1])) +
              _t_points[j](0) / 4.0 * _thickness[i] * (_node_normal[2] - _node_normal[1]);

      updateGVectors(); // for large strain problems

      // corresponding to strain(0,2)
      for (unsigned int component = 0; component < 3; component++)
      {
        (*_B[j])[i](3, 2 + component * 4) = 0.125 * (1.0 + _2d_points[i](1)) * _g3_a(component);
        (*_B[j])[i](3, 3 + component * 4) = 0.125 * (1.0 + _2d_points[i](1)) * -_g3_a(component);
        (*_B[j])[i](3, 1 + component * 4) = 0.125 * (1.0 - _2d_points[i](1)) * _g3_c(component);
        (*_B[j])[i](3, component * 4) = 0.125 * (1.0 - _2d_points[i](1)) * -_g3_c(component);
      }
      (*_B[j])[i](3, 14) = 0.125 * (1.0 + _2d_points[i](1)) * 0.5 * _thickness[i] * _g1_a * -_v2[2];
      (*_B[j])[i](3, 18) = 0.125 * (1.0 + _2d_points[i](1)) * 0.5 * _thickness[i] * _g1_a * _v1[2];
      (*_B[j])[i](3, 15) = 0.125 * (1.0 + _2d_points[i](1)) * 0.5 * _thickness[i] * _g1_a * -_v2[3];
      (*_B[j])[i](3, 19) = 0.125 * (1.0 + _2d_points[i](1)) * 0.5 * _thickness[i] * _g1_a * _v1[3];

      (*_B[j])[i](3, 13) = 0.125 * (1.0 - _2d_points[i](1)) * 0.5 * _thickness[i] * _g1_c * -_v2[1];
      (*_B[j])[i](3, 17) = 0.125 * (1.0 - _2d_points[i](1)) * 0.5 * _thickness[i] * _g1_c * _v1[1];
      (*_B[j])[i](3, 12) = 0.125 * (1.0 - _2d_points[i](1)) * 0.5 * _thickness[i] * _g1_c * -_v2[0];
      (*_B[j])[i](3, 16) = 0.125 * (1.0 - _2d_points[i](1)) * 0.5 * _thickness[i] * _g1_c * _v1[0];

      // corresponding to strain(1,2)
      for (unsigned int component = 0; component < 3; component++)
      {
        (*_B[j])[i](4, 2 + component * 4) = 0.125 * (1.0 + _2d_points[i](0)) * _g3_d(component);
        (*_B[j])[i](4, 1 + component * 4) = 0.125 * (1.0 + _2d_points[i](0)) * -_g3_d(component);
        (*_B[j])[i](4, 3 + component * 4) = 0.125 * (1.0 - _2d_points[i](0)) * _g3_b(component);
        (*_B[j])[i](4, component * 4) = 0.125 * (1.0 - _2d_points[i](0)) * -_g3_b(component);
      }
      (*_B[j])[i](4, 14) = 0.125 * (1.0 + _2d_points[i](0)) * 0.5 * _thickness[i] * _g2_d * -_v2[2];
      (*_B[j])[i](4, 18) = 0.125 * (1.0 + _2d_points[i](0)) * 0.5 * _thickness[i] * _g2_d * _v1[2];
      (*_B[j])[i](4, 13) = 0.125 * (1.0 + _2d_points[i](0)) * 0.5 * _thickness[i] * _g2_d * -_v2[1];
      (*_B[j])[i](4, 17) = 0.125 * (1.0 + _2d_points[i](0)) * 0.5 * _thickness[i] * _g2_d * _v1[1];

      (*_B[j])[i](4, 15) = 0.125 * (1.0 - _2d_points[i](0)) * 0.5 * _thickness[i] * _g2_b * -_v2[3];
      (*_B[j])[i](4, 19) = 0.125 * (1.0 - _2d_points[i](0)) * 0.5 * _thickness[i] * _g2_b * _v1[3];
      (*_B[j])[i](4, 12) = 0.125 * (1.0 - _2d_points[i](0)) * 0.5 * _thickness[i] * _g2_b * -_v2[0];
      (*_B[j])[i](4, 16) = 0.125 * (1.0 - _2d_points[i](0)) * 0.5 * _thickness[i] * _g2_b * _v1[0];
    }
  }
}

void
ADComputeIncrementalShellStrain::computeSolnVector()
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
      if (ADReal::do_derivatives)
        Moose::derivInsert(
            _soln_vector(j + i * _nodes.size()).derivatives(), _soln_disp_index[j][i], 1.);
    }

    for (unsigned int i = 0; i < _nrot; ++i)
    {
      _soln_rot_index[j][i] = _nodes[j]->dof_number(_nonlinear_sys.number(), _rot_num[i], 0);
      _soln_vector(j + 12 + i * _nodes.size()) =
          (*_sol)(_soln_rot_index[j][i]) - _sol_old(_soln_rot_index[j][i]);
      if (ADReal::do_derivatives)
        Moose::derivInsert(
            _soln_vector(j + 12 + i * _nodes.size()).derivatives(), _soln_rot_index[j][i], 1.);
    }
  }
}
