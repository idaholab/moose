//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADComputeFiniteShellStrain.h"
#include "DenseMatrix.h"

#include "libmesh/quadrature.h"
#include "libmesh/utility.h"
#include "libmesh/enum_quadrature_type.h"
#include "libmesh/fe_type.h"
#include "libmesh/string_to_enum.h"
#include "libmesh/quadrature_gauss.h"

registerMooseObject("TensorMechanicsApp", ADComputeFiniteShellStrain);

InputParameters
ADComputeFiniteShellStrain::validParams()
{
  InputParameters params = ADComputeIncrementalShellStrain::validParams();
  params.addClassDescription("Compute a large strain increment for the shell.");
  return params;
}

ADComputeFiniteShellStrain::ADComputeFiniteShellStrain(const InputParameters & parameters)
  : ADComputeIncrementalShellStrain(parameters), _B_nl()
{
  _B_nl.resize(_t_points.size());

  for (unsigned int i = 0; i < _t_points.size(); ++i)
    _B_nl[i] = &declareADProperty<DenseMatrix<Real>>("B_nl_t_points_" + std::to_string(i));
}

void
ADComputeFiniteShellStrain::initQpStatefulProperties()
{
  computeGMatrix();

  ADDenseMatrix b(5, 20);
  for (unsigned int t = 0; t < _t_points.size(); ++t)
    (*_B_nl[t])[_qp] = b;
}

void
ADComputeFiniteShellStrain::computeProperties()
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

  for (unsigned int i = 0; i < _2d_points.size(); ++i)
  {
    for (unsigned int j = 0; j < _t_points.size(); ++j)
    {
      (*_ge[j])[i] = (*_ge_old[j])[i];
      (*_J_map[j])[i] = (*_J_map_old[j])[i];
    }
  }

  computeSolnVector();
  updatedxyz();

  for (unsigned int k = 0; k < _nodes.size(); ++k)
    _node_normal[k] = _node_normal_old[k];

  computeBMatrix();

  computeNodeNormal();

  computeBNLMatrix();

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
}

void
ADComputeFiniteShellStrain::computeNodeNormal()
{
  // update _node_normal
  for (unsigned int k = 0; k < _nodes.size(); ++k)
  {
    _node_normal[k] =
        -_v2[k] * _soln_vector(12 + k) + _v1[k] * _soln_vector(16 + k) + _node_normal_old[k];
    _node_normal[k] /= _node_normal[k].norm();
  }
}

void
ADComputeFiniteShellStrain::updatedxyz()
{
  for (unsigned int i = 0; i < _2d_points.size(); ++i)
  {
    for (unsigned int j = 0; j < _t_points.size(); ++j)
    {
      for (unsigned int component = 0; component < 3; ++component)
      {
        (*_dxyz_dxi[j])[i](component) = 0.0;
        (*_dxyz_deta[j])[i](component) = 0.0;
        (*_dxyz_dzeta[j])[i](component) = 0.0;
        for (unsigned int k = 0; k < _nodes.size(); ++k)
        {
          (*_dxyz_dxi[j])[i](component) +=
              _dphidxi_map[k][i] *
                  ((*_nodes[k])(component) + _sol_old(_soln_disp_index[k][component])) +
              _t_points[j](0) / 2.0 * _thickness[i] * _dphidxi_map[k][i] *
                  _node_normal_old[k](component);
          (*_dxyz_deta[j])[i](component) +=
              _dphideta_map[k][i] *
                  ((*_nodes[k])(component) + _sol_old(_soln_disp_index[k][component])) +
              _t_points[j](0) / 2.0 * _thickness[i] * _dphideta_map[k][i] *
                  _node_normal_old[k](component);
          (*_dxyz_dzeta[j])[i](component) +=
              _thickness[i] * _phi_map[k][i] * _node_normal_old[k](component) / 2.0;
        }
      }
    }
  }
}

void
ADComputeFiniteShellStrain::updateGVectors()
{
  for (unsigned int component = 0; component < 3; ++component)
  {
    _g1_a(component) +=
        0.5 * (_sol_old(_soln_disp_index[2][component]) - _sol_old(_soln_disp_index[3][component]));
    _g1_c(component) +=
        0.5 * (_sol_old(_soln_disp_index[1][component]) - _sol_old(_soln_disp_index[0][component]));
    _g2_b(component) +=
        0.5 * (_sol_old(_soln_disp_index[3][component]) - _sol_old(_soln_disp_index[0][component]));
    _g2_d(component) +=
        0.5 * (_sol_old(_soln_disp_index[2][component]) - _sol_old(_soln_disp_index[1][component]));
  }
}

void
ADComputeFiniteShellStrain::computeBNLMatrix()
{
  // compute BNL matrix - rows correspond to [ux1, ux2, ux3, ux4, uy1, uy2, uy3, uy4, uz1, uz2, uz3,
  // uz4, a1, a2, a3, a4, b1, b2, b3, b4]

  for (unsigned int i = 0; i < _2d_points.size(); ++i)
  {
    for (unsigned int j = 0; j < _t_points.size(); ++j)
    {
      (*_B_nl[j])[i].resize(5, 20);
      (*_B_nl[j])[i].zero();
      for (unsigned int k = 0; k < 4; ++k)
      {
        for (unsigned int p = 0; p < 4; ++p) // loop over nodes
        {
          // corresponding to strain(0,0)
          (*_B_nl[j])[i](0, k) += _dphidxi_map[k][i] * _dphidxi_map[p][i] *
                                  (_soln_vector(p) + _t_points[j](0) / 2.0 * _thickness[i] *
                                                         (-_soln_vector(p + 12) * _v2[p](0) +
                                                          _soln_vector(p + 16) * _v1[p](0)));
          (*_B_nl[j])[i](0, 4 + k) +=
              _dphidxi_map[k][i] * _dphidxi_map[p][i] *
              (_soln_vector(p + 4) +
               _t_points[j](0) / 2.0 * _thickness[i] *
                   (-_soln_vector(p + 12) * _v2[p](1) + _soln_vector(p + 16) * _v1[p](1)));
          (*_B_nl[j])[i](0, 8 + k) +=
              _dphidxi_map[k][i] * _dphidxi_map[p][i] *
              (_soln_vector(p + 8) +
               _t_points[j](0) / 2.0 * _thickness[i] *
                   (-_soln_vector(p + 12) * _v2[p](2) + _soln_vector(p + 16) * _v1[p](2)));
          (*_B_nl[j])[i](0, 12 + k) +=
              _t_points[j](0) / 2.0 * _thickness[i] * _dphidxi_map[k][i] * _dphidxi_map[p][i] *
              (-(_v2[p](0) * _soln_vector(p) + _v2[p](1) * _soln_vector(p + 4) +
                 _v2[p](2) * _soln_vector(p + 8)) +
               _t_points[j](0) / 2.0 * _thickness[i] * _v2[k] *
                   (_v2[p] * _soln_vector(p + 12) - _v1[p] * _soln_vector(p + 16)));
          (*_B_nl[j])[i](0, 16 + k) +=
              _t_points[j](0) / 2.0 * _thickness[i] * _dphidxi_map[k][i] * _dphidxi_map[p][i] *
              ((_v1[p](0) * _soln_vector(p) + _v1[p](1) * _soln_vector(p + 4) +
                _v1[p](2) * _soln_vector(p + 8)) +
               _t_points[j](0) / 2.0 * _thickness[i] * _v1[k] *
                   (-_v2[p] * _soln_vector(p + 12) + _v1[p] * _soln_vector(p + 16)));

          // corresponding to strain(1,1)
          (*_B_nl[j])[i](1, k) += _dphideta_map[k][i] * _dphideta_map[p][i] *
                                  (_soln_vector(p) + _t_points[j](0) / 2.0 * _thickness[i] *
                                                         (-_soln_vector(p + 12) * _v2[p](0) +
                                                          _soln_vector(p + 16) * _v1[p](0)));
          (*_B_nl[j])[i](1, 4 + k) +=
              _dphideta_map[k][i] * _dphideta_map[p][i] *
              (_soln_vector(p + 4) +
               _t_points[j](0) / 2.0 * _thickness[i] *
                   (-_soln_vector(p + 12) * _v2[p](1) + _soln_vector(p + 16) * _v1[p](1)));
          (*_B_nl[j])[i](1, 8 + k) +=
              _dphideta_map[k][i] * _dphideta_map[p][i] *
              (_soln_vector(p + 8) +
               _t_points[j](0) / 2.0 * _thickness[i] *
                   (-_soln_vector(p + 12) * _v2[p](2) + _soln_vector(p + 16) * _v1[p](2)));
          (*_B_nl[j])[i](1, 12 + k) +=
              _t_points[j](0) / 2.0 * _thickness[i] * _dphideta_map[k][i] * _dphideta_map[p][i] *
              (-(_v2[p](0) * _soln_vector(p) + _v2[p](1) * _soln_vector(p + 4) +
                 _v2[p](2) * _soln_vector(p + 8)) +
               _t_points[j](0) / 2.0 * _thickness[i] * _v2[k] *
                   (_v2[p] * _soln_vector(p + 12) - _v1[p] * _soln_vector(p + 16)));
          (*_B_nl[j])[i](1, 16 + k) +=
              _t_points[j](0) / 2.0 * _thickness[i] * _dphideta_map[k][i] * _dphideta_map[p][i] *
              ((_v1[p](0) * _soln_vector(p) + _v1[p](1) * _soln_vector(p + 4) +
                _v1[p](2) * _soln_vector(p + 8)) +
               _t_points[j](0) / 2.0 * _thickness[i] * _v1[k] *
                   (-_v2[p] * _soln_vector(p + 12) + _v1[p] * _soln_vector(p + 16)));

          // terms corresponding to strain(2,2) are 0.

          // corresponding to strain(0,1)
          (*_B_nl[j])[i](2, k) += 0.5 *
                                  (_dphidxi_map[k][i] * _dphideta_map[p][i] +
                                   _dphideta_map[k][i] * _dphidxi_map[p][i]) *
                                  (_soln_vector(p) + _t_points[j](0) / 2.0 * _thickness[i] *
                                                         (-_soln_vector(p + 12) * _v2[p](0) +
                                                          _soln_vector(p + 16) * _v1[p](0)));
          (*_B_nl[j])[i](2, 4 + k) +=
              0.5 *
              (_dphidxi_map[k][i] * _dphideta_map[p][i] +
               _dphideta_map[k][i] * _dphidxi_map[p][i]) *
              (_soln_vector(p + 4) +
               _t_points[j](0) / 2.0 * _thickness[i] *
                   (-_soln_vector(p + 12) * _v2[p](1) + _soln_vector(p + 16) * _v1[p](1)));
          (*_B_nl[j])[i](2, 8 + k) +=
              0.5 *
              (_dphidxi_map[k][i] * _dphideta_map[p][i] +
               _dphideta_map[k][i] * _dphidxi_map[p][i]) *
              (_soln_vector(p + 8) +
               _t_points[j](0) / 2.0 * _thickness[i] *
                   (-_soln_vector(p + 12) * _v2[p](2) + _soln_vector(p + 16) * _v1[p](2)));
          (*_B_nl[j])[i](2, 12 + k) +=
              _t_points[j](0) * 0.25 *
              (_dphidxi_map[k][i] * _dphideta_map[p][i] +
               _dphideta_map[k][i] * _dphidxi_map[p][i]) *
              _thickness[i] *
              (-(_v2[k](0) * _soln_vector(p) + _v2[k](1) * _soln_vector(p + 4) +
                 _v2[k](2) * _soln_vector(p + 8)) +
               _t_points[j](0) / 2.0 * _thickness[i] * _v2[k] *
                   (_v2[p] * _soln_vector(p + 12) - _v1[p] * _soln_vector(p + 16)));
          (*_B_nl[j])[i](2, 16 + k) +=
              _t_points[j](0) * 0.25 *
              (_dphidxi_map[k][i] * _dphideta_map[p][i] +
               _dphideta_map[k][i] * _dphidxi_map[p][i]) *
              _thickness[i] *
              ((_v1[k](0) * _soln_vector(p) + _v1[k](1) * _soln_vector(p + 4) +
                _v1[k](2) * _soln_vector(p + 8)) +
               _t_points[j](0) / 2.0 * _thickness[i] * _v1[k] *
                   (-_v2[p] * _soln_vector(p + 12) + _v1[p] * _soln_vector(p + 16)));
        }
      }

      for (unsigned int component = 0; component < 3; ++component)
      {
        // corresponding to strain(0,2)
        (*_B_nl[j])[i](3, 2 + component * 4) +=
            1.0 / 32.0 * (1.0 + _2d_points[i](1)) * _thickness[i] *
            (-_soln_vector(12 + 2) * _v2[2](component) + _soln_vector(16 + 2) * _v1[2](component) -
             _soln_vector(12 + 3) * _v2[3](component) + _soln_vector(16 + 3) * _v1[3](component));
        (*_B_nl[j])[i](3, 3 + component * 4) += -(*_B_nl[j])[i](3, 2 + component * 4);

        (*_B_nl[j])[i](3, 1 + component * 4) +=
            1.0 / 32.0 * (1.0 - _2d_points[i](1)) * _thickness[i] *
            (-_soln_vector(12 + 1) * _v2[1](component) + _soln_vector(16 + 1) * _v1[1](component) -
             _soln_vector(12 + 0) * _v2[0](component) + _soln_vector(16 + 0) * _v1[0](component));
        (*_B_nl[j])[i](3, component * 4) += -(*_B_nl[j])[i](3, 1 + component * 4);

        // adding contributions corresponding to alpha 2 and 3 and beta 2 and 3
        (*_B_nl[j])[i](3, 12 + 2) +=
            -1.0 / 32.0 * (1.0 + _2d_points[i](1)) * _thickness[i] * _v2[2](component) *
            (_soln_vector(2 + component * 4) - _soln_vector(3 + component * 4));
        (*_B_nl[j])[i](3, 16 + 2) +=
            1.0 / 32.0 * (1.0 + _2d_points[i](1)) * _thickness[i] * _v1[2](component) *
            (_soln_vector(2 + component * 4) - _soln_vector(3 + component * 4));
        (*_B_nl[j])[i](3, 12 + 3) +=
            -1.0 / 32.0 * (1.0 + _2d_points[i](1)) * _thickness[i] * _v2[3](component) *
            (_soln_vector(2 + component * 4) - _soln_vector(3 + component * 4));
        (*_B_nl[j])[i](3, 16 + 3) +=
            1.0 / 32.0 * (1.0 + _2d_points[i](1)) * _thickness[i] * _v1[3](component) *
            (_soln_vector(2 + component * 4) - _soln_vector(3 + component * 4));

        // adding contributions corresponding to alpha 1 and 0 and beta 1 and 0
        (*_B_nl[j])[i](3, 12 + 1) +=
            -1.0 / 32.0 * (1.0 - _2d_points[i](1)) * _thickness[i] * _v2[1](component) *
            (_soln_vector(1 + component * 4) - _soln_vector(component * 4));
        (*_B_nl[j])[i](3, 16 + 1) +=
            1.0 / 32.0 * (1.0 - _2d_points[i](1)) * _thickness[i] * _v1[1](component) *
            (_soln_vector(1 + component * 4) - _soln_vector(component * 4));
        (*_B_nl[j])[i](3, 12 + 0) +=
            -1.0 / 32.0 * (1.0 - _2d_points[i](1)) * _thickness[i] * _v2[0](component) *
            (_soln_vector(1 + component * 4) - _soln_vector(component * 4));
        (*_B_nl[j])[i](3, 16 + 0) +=
            1.0 / 32.0 * (1.0 - _2d_points[i](1)) * _thickness[i] * _v1[0](component) *
            (_soln_vector(1 + component * 4) - _soln_vector(component * 4));

        // corresponding to strain(1,2)
        (*_B_nl[j])[i](4, 2 + component * 4) +=
            1.0 / 32.0 * (1.0 + _2d_points[i](0)) * _thickness[i] *
            (-_soln_vector(12 + 2) * _v2[2](component) + _soln_vector(16 + 2) * _v1[2](component) -
             _soln_vector(12 + 1) * _v2[1](component) + _soln_vector(16 + 1) * _v1[1](component));
        (*_B_nl[j])[i](4, 1 + component * 4) += -(*_B_nl[j])[i](3, 2 + component * 4);

        (*_B_nl[j])[i](4, 3 + component * 4) +=
            1.0 / 32.0 * (1.0 - _2d_points[i](0)) * _thickness[i] *
            (-_soln_vector(12 + 3) * _v2[3](component) + _soln_vector(16 + 3) * _v1[3](component) -
             _soln_vector(12 + 0) * _v2[0](component) + _soln_vector(16 + 0) * _v1[0](component));
        (*_B_nl[j])[i](4, component * 4) += -(*_B_nl[j])[i](3, 3 + component * 4);

        // adding contributions corresponding to alpha 2, 1 and beta 2 , 1
        (*_B_nl[j])[i](4, 12 + 2) +=
            -1.0 / 32.0 * (1.0 + _2d_points[i](0)) * _thickness[i] * _v2[2](component) *
            (_soln_vector(2 + component * 4) - _soln_vector(1 + component * 4));
        (*_B_nl[j])[i](4, 16 + 2) +=
            1.0 / 32.0 * (1.0 + _2d_points[i](0)) * _thickness[i] * _v1[2](component) *
            (_soln_vector(2 + component * 4) - _soln_vector(1 + component * 4));
        (*_B_nl[j])[i](4, 12 + 1) +=
            -1.0 / 32.0 * (1.0 + _2d_points[i](0)) * _thickness[i] * _v2[1](component) *
            (_soln_vector(2 + component * 4) - _soln_vector(1 + component * 4));
        (*_B_nl[j])[i](4, 16 + 1) +=
            1.0 / 32.0 * (1.0 + _2d_points[i](0)) * _thickness[i] * _v1[1](component) *
            (_soln_vector(2 + component * 4) - _soln_vector(1 + component * 4));

        // adding contributions corresponding to alpha 3, 0 and beta 3 , 0
        (*_B_nl[j])[i](4, 12 + 3) +=
            -1.0 / 32.0 * (1.0 - _2d_points[i](0)) * _thickness[i] * _v2[3](component) *
            (_soln_vector(3 + component * 4) - _soln_vector(component * 4));
        (*_B_nl[j])[i](4, 16 + 3) +=
            1.0 / 32.0 * (1.0 - _2d_points[i](0)) * _thickness[i] * _v1[3](component) *
            (_soln_vector(3 + component * 4) - _soln_vector(component * 4));
        (*_B_nl[j])[i](4, 12 + 0) +=
            -1.0 / 32.0 * (1.0 - _2d_points[i](0)) * _thickness[i] * _v2[0](component) *
            (_soln_vector(3 + component * 4) - _soln_vector(component * 4));
        (*_B_nl[j])[i](4, 16 + 0) +=
            1.0 / 32.0 * (1.0 - _2d_points[i](0)) * _thickness[i] * _v1[0](component) *
            (_soln_vector(3 + component * 4) - _soln_vector(component * 4));
      }
    }
  }
}
