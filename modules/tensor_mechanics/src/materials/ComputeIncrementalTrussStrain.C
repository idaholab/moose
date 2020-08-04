//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeIncrementalTrussStrain.h"
#include "MooseMesh.h"
#include "Assembly.h"
#include "NonlinearSystem.h"
#include "MooseVariable.h"
#include "Function.h"

#include "libmesh/quadrature.h"
#include "libmesh/utility.h"

#include "Material.h"
#include "MooseVariable.h"

registerMooseObject("TensorMechanicsApp", ComputeIncrementalTrussStrain);

InputParameters
ComputeIncrementalTrussStrain::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Compute a infinitesimal/large strain increment for the truss.");
  params.addParam<std::string>("base_name",
                               "Optional parameter that allows the user to define "
                               "multiple mechanics material systems on the same "
                               "block, i.e. for multiple phases");
  params.addRequiredCoupledVar(
      "displacements",
      "The displacements appropriate for the simulation geometry and coordinate system");
  params.addRequiredParam<RealGradient>("y_orientation",
                                        "Orientation of the y direction along "
                                        "with Iyy is provided. This should be "
                                        "perpendicular to the axis of the beam.");
  params.addCoupledVar("youngs_modulus", "Variable containing Young's modulus");
  params.addRequiredCoupledVar(
      "area",
      "Cross-section area of the truss. Can be supplied as either a number or a variable name.");
  params.addParam<bool>("large_strain", false, "Set to true if large strain are to be calculated.");
  params.addParam<std::vector<MaterialPropertyName>>(
      "eigenstrain_names", "List of truss eigenstrains to be applied in this strain calculation.");
  params.addParam<FunctionName>(
      "elasticity_prefactor",
      "Optional function to use as a scalar prefactor on the elasticity vector for the truss.");
  return params;
}

ComputeIncrementalTrussStrain::ComputeIncrementalTrussStrain(const InputParameters & parameters)
  : Material(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    // _youngs_modulus(coupledValue("youngs_modulus")),
    _total_stretch(declareProperty<Real>(_base_name + "total_stretch")),
    _elastic_stretch(declareProperty<Real>(_base_name + "elastic_stretch")),
    // _axial_stress(declareProperty<Real>(_base_name + "axial_stress")),
    // _e_over_l(declareProperty<Real>(_base_name + "e_over_l")),

    _ndisp(coupledComponents("displacements")),
    _disp_num(_ndisp),
    _area(coupledValue("area")),

    _original_length(declareProperty<Real>("original_length")),
    _total_rotation(declareProperty<RankTwoTensor>("total_rotation")),
    _total_disp_strain(declareProperty<RealVectorValue>("total_disp_strain")),
    _total_disp_strain_old(getMaterialPropertyOld<RealVectorValue>("total_disp_strain")),
    _mech_disp_strain_increment(declareProperty<RealVectorValue>("mech_disp_strain_increment")),
    //
    _material_stiffness(getMaterialPropertyByName<RealVectorValue>("material_stiffness")),
    _K11(declareProperty<RankTwoTensor>("Jacobian_11")),
    _large_strain(getParam<bool>("large_strain")),
    _eigenstrain_names(getParam<std::vector<MaterialPropertyName>>("eigenstrain_names")),
    _disp_eigenstrain(_eigenstrain_names.size()),
    _disp_eigenstrain_old(_eigenstrain_names.size()),
    _nonlinear_sys(_fe_problem.getNonlinearSystemBase()),
    _initial_rotation(declareProperty<RankTwoTensor>("initial_rotation")),
    _effective_stiffness(declareProperty<Real>("effective_stiffness")),
    _prefactor_function(isParamValid("elasticity_prefactor") ? &getFunction("elasticity_prefactor")
                                                             : nullptr)
{
  const std::vector<VariableName> & nl_vnames(getParam<std::vector<VariableName>>("displacements"));
  _ndisp = nl_vnames.size();
  // _disp_num = _ndisp;

  // fetch coupled variables and gradients (as stateful properties if necessary)
  for (unsigned int i = 0; i < _ndisp; ++i)
  {
    // MooseVariable * disp_variable = getVar("displacements", i);
    // _disp_num[i] = disp_variable->number();

    // fetch nonlinear variables
    _disp_var.push_back(&_fe_problem.getStandardVariable(_tid, nl_vnames[i]));
  }

  if (_large_strain)
    mooseError("ComputeIncrementalTrussStrain: Large strain calculation does not currently "
               "support asymmetric truss configurations with non-zero first or third moments of "
               "area.");

  for (unsigned int i = 0; i < _eigenstrain_names.size(); ++i)
  {
    _disp_eigenstrain[i] = &getMaterialProperty<RealVectorValue>("disp_" + _eigenstrain_names[i]);
    _disp_eigenstrain_old[i] =
        &getMaterialPropertyOld<RealVectorValue>("disp_" + _eigenstrain_names[i]);
  }
}

void
ComputeIncrementalTrussStrain::initQpStatefulProperties()
{
  _total_stretch[_qp] = 0.0;
  _elastic_stretch[_qp] = 0.0;

  // out<<" in ComputeIncrementalTrussStrain" << std::endl;


  // compute initial orientation of the truss for calculating initial rotation matrix
  const std::vector<RealGradient> * orientation =
      &_subproblem.assembly(_tid).getFE(FEType(), 1)->get_dxyzdxi();
  RealGradient x_orientation = (*orientation)[0];
  x_orientation /= x_orientation.norm();

  RealGradient y_orientation = getParam<RealGradient>("y_orientation");
  y_orientation /= y_orientation.norm();
  Real sum = x_orientation(0) * y_orientation(0) + x_orientation(1) * y_orientation(1) +
             x_orientation(2) * y_orientation(2);

  if (std::abs(sum) > 1e-4)
    mooseError("ComputeIncrementalTrussStrain: y_orientation should be perpendicular to "
               "the axis of the truss.");

  // Calculate z orientation as a cross product of the x and y orientations
  RealGradient z_orientation;
  z_orientation(0) = (x_orientation(1) * y_orientation(2) - x_orientation(2) * y_orientation(1));
  z_orientation(1) = (x_orientation(2) * y_orientation(0) - x_orientation(0) * y_orientation(2));
  z_orientation(2) = (x_orientation(0) * y_orientation(1) - x_orientation(1) * y_orientation(0));

  // Rotation matrix from global to original truss local configuration
  _original_local_config(0, 0) = x_orientation(0);
  _original_local_config(0, 1) = x_orientation(1);
  _original_local_config(0, 2) = x_orientation(2);
  _original_local_config(1, 0) = y_orientation(0);
  _original_local_config(1, 1) = y_orientation(1);
  _original_local_config(1, 2) = y_orientation(2);
  _original_local_config(2, 0) = z_orientation(0);
  _original_local_config(2, 1) = z_orientation(1);
  _original_local_config(2, 2) = z_orientation(2);

  _total_rotation[_qp] = _original_local_config;

  RealVectorValue temp;
  _total_disp_strain[_qp] = temp;
}

void
ComputeIncrementalTrussStrain::computeProperties()
{
  // check for consistency of the number of element nodes
  mooseAssert(_current_elem->n_nodes() == 2, "Truss element needs to have exactly two nodes.");

  // fetch the two end nodes for current element
  std::vector<const Node *> node;
  for (unsigned int i = 0; i < 2; ++i)
    node.push_back(_current_elem->node_ptr(i));

  // calculate original length of a truss element
  RealGradient dxyz;
  for (unsigned int i = 0; i < _ndisp; ++i)
    dxyz(i) = (*node[1])(i) - (*node[0])(i);
  _original_length[0] = dxyz.norm();

  // Fetch the solution for the two end nodes at time t
  // const NumericVector<Number> & sol = *_nonlinear_sys.currentSolution();
  // const NumericVector<Number> & sol_old = _nonlinear_sys.solutionOld();
  NonlinearSystemBase & nonlinear_sys = _fe_problem.getNonlinearSystemBase();
  const NumericVector<Number> & sol = *nonlinear_sys.currentSolution();

  std::vector<Real> disp0, disp1;
  for (unsigned int i = 0; i < _ndisp; ++i)
  {
    disp0.push_back(sol(node[0]->dof_number(nonlinear_sys.number(), _disp_var[i]->number(), 0)));
    disp1.push_back(sol(node[1]->dof_number(nonlinear_sys.number(), _disp_var[i]->number(), 0)));
  }
  // calculate current length of a truss element
  for (unsigned int i = 0; i < _ndisp; ++i)
    dxyz(i) += disp1[i] - disp0[i];
  _current_length = dxyz.norm();

  // for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
  // {
  //   // _e_over_l[_qp] = _material_stiffness[0](0) / _origin_length;
  //
  //   // computeQpStrain();
  //   // computeQpStress();
  // }
/*
  for (unsigned int i = 0; i < _ndisp; ++i)
  {
    _soln_disp_index_0[i] = node[0]->dof_number(_nonlinear_sys.number(), _disp_num[i], 0);
    _soln_disp_index_1[i] = node[1]->dof_number(_nonlinear_sys.number(), _disp_num[i], 0);

    _disp0(i) = sol(_soln_disp_index_0[i]) - sol_old(_soln_disp_index_0[i]);
    _disp1(i) = sol(_soln_disp_index_1[i]) - sol_old(_soln_disp_index_1[i]);
  }
*/

  // For small rotation problems, the rotation matrix is essentially the transformation from the
  // global to original truss local configuration and is never updated. This method has to be
  // overriden for scenarios with finite rotation
  computeRotation();
  // _initial_rotation[0] = _original_local_config;

  // for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
  //   computeQpStrain();
  //
  // if (_fe_problem.currentlyComputingJacobian())
  //   computeStiffnessMatrix();
}

void
ComputeIncrementalTrussStrain::computeQpStrain()
{
  _total_stretch[_qp] = _current_length / _origin_length - 1.0;
  _elastic_stretch[_qp] = _total_stretch[_qp];
  // _elastic_stretch[_qp] = _total_stretch[_qp] - _thermal_expansion_coeff * (_T[_qp] - _T0);

/*
  // const Real A_avg = (_area[0] + _area[1]) / 2.0;

  // Rotate the gradient of displacements and rotations at t+delta t from global coordinate
  // frame to truss local coordinate frame
  const RealVectorValue grad_disp_0(1.0 / _original_length[0] * (_disp1 - _disp0));
  const RealVectorValue grad_rot_0(1.0 / _original_length[0] * (_rot1 - _rot0));
  const RealVectorValue avg_rot(
      0.5 * (_rot0(0) + _rot1(0)), 0.5 * (_rot0(1) + _rot1(1)), 0.5 * (_rot0(2) + _rot1(2)));

  _grad_disp_0_local_t = _total_rotation[0] * grad_disp_0;
  _grad_rot_0_local_t = _total_rotation[0] * grad_rot_0;
  _avg_rot_local_t = _total_rotation[0] * avg_rot;


  // displacement at any location on truss in local coordinate system at t
  // u_1 = u_n1 - rot_3 * y + rot_2 * z
  // u_2 = u_n2 - rot_1 * z
  // u_3 = u_n3 + rot_1 * y
  // where u_n1, u_n2, u_n3 are displacements at neutral axis

  // small strain
  // e_11 = u_1,1 = u_n1, 1 - rot_3, 1 * y + rot_2, 1 * z
  // e_12 = 2 * 0.5 * (u_1,2 + u_2,1) = (- rot_3 + u_n2,1 - rot_1,1 * z)
  // e_13 = 2 * 0.5 * (u_1,3 + u_3,1) = (rot_2 + u_n3,1 + rot_1,1 * y)

  // axial strain at each qp along the length of the truss
  _mech_disp_strain_increment[_qp](0) = _grad_disp_0_local_t(0) * _area[_qp];
  _mech_disp_strain_increment[_qp](1) = 0;
  _mech_disp_strain_increment[_qp](2) = 0;

  if (_large_strain)
  {
    _mech_disp_strain_increment[_qp](0) +=
        0.5 *
        ((Utility::pow<2>(_grad_disp_0_local_t(0)) + Utility::pow<2>(_grad_disp_0_local_t(1)) +
          Utility::pow<2>(_grad_disp_0_local_t(2))) *
             _area[_qp]);
    _mech_disp_strain_increment[_qp](1) += (-_avg_rot_local_t(2) * _grad_disp_0_local_t(0) +
                                            _avg_rot_local_t(0) * _grad_disp_0_local_t(2)) *
                                           _area[_qp];
    _mech_disp_strain_increment[_qp](2) += (_avg_rot_local_t(1) * _grad_disp_0_local_t(0) -
                                            _avg_rot_local_t(0) * _grad_disp_0_local_t(1)) *
                                           _area[_qp];
  }

  _total_disp_strain[_qp] = _total_rotation[0].transpose() * _mech_disp_strain_increment[_qp] +
                            _total_disp_strain_old[_qp];

  // Convert eigenstrain increment from global to truss local coordinate system and remove eigen
  // strain increment
  for (unsigned int i = 0; i < _eigenstrain_names.size(); ++i)
  {
    _mech_disp_strain_increment[_qp] -=
        _total_rotation[0] * ((*_disp_eigenstrain[i])[_qp] - (*_disp_eigenstrain_old[i])[_qp]) *
        _area[_qp];
  }

  // Real c1_paper = std::sqrt(_material_stiffness[0](0));
  // Real c2_paper = std::sqrt(_material_stiffness[0](1));
  //
  // Real effec_stiff_1 = std::max(c1_paper, c2_paper);
  //
  // Real effec_stiff_2 = 2 / (c2_paper * std::sqrt(A_avg / Iz_avg));
  //
  // _effective_stiffness[_qp] = std::max(effec_stiff_1, _original_length[0] / effec_stiff_2);
  //
  // if (_prefactor_function)
  //   _effective_stiffness[_qp] *= std::sqrt(_prefactor_function->value(_t, _q_point[_qp]));
  */
}

void
ComputeIncrementalTrussStrain::computeStiffnessMatrix()
{
  // _e_over_l[_qp] = _material_stiffness[0](0) / _origin_length;

  const Real youngs_modulus = _material_stiffness[0](0);
  const Real A_avg = (_area[0] + _area[1]) / 2.0;

  Real K11_local = youngs_modulus * A_avg / _original_length[0];
  _K11[0] = _total_rotation[0].transpose() * K11_local * _total_rotation[0];

  out << " stiffness K11 " << _total_rotation[0].transpose() * K11_local * _total_rotation[0] << std::endl;

  // _K11[0] = K11_local;

  // RankTwoTensor K11_local;
  // K11_local.zero();
  // // K11_local(0, 0) = youngs_modulus * A_avg / _original_length[0];
  // // K11_local(1, 1) = shear_modulus * A_avg / _original_length[0];
  // // K11_local(2, 2) = shear_modulus * A_avg / _original_length[0];
  // _K11[0] = _total_rotation[0].transpose() * K11_local * _total_rotation[0];

  // _K12[0] = -1.0 * K11_local;
  // _K21[0] = -1.0 * K11_local;
  // _K22[0] = K11_local;

/*
  const Real A_avg = (_area[0] + _area[1]) / 2.0;

  // K = |K11 K12|
  //     |K21 K22|

  // relation between translational displacements at node 0 and translational forces at node 0
  RankTwoTensor K11_local;
  K11_local.zero();
  K11_local(0, 0) = youngs_modulus * A_avg / _original_length[0];
  // K11_local(1, 1) = shear_modulus * A_avg / _original_length[0];
  // K11_local(2, 2) = shear_modulus * A_avg / _original_length[0];
  K11_local(1, 1) = 0;
  K11_local(2, 2) = 0;
  // _K11[0] = _total_rotation[0].transpose() * K11_local * _total_rotation[0];
  _K11[0] = K11_local;



  // stiffness matrix for large strain
  if (_large_strain)
  {
    // k1_large is the stiffness matrix obtained from sigma_xx * d(epsilon_xx)
    RankTwoTensor k1_large_11;
    // row 1
    k1_large_11(0, 0) = Utility::pow<2>(_grad_disp_0_local_t(0)) +
                        1.5 * Utility::pow<2>(_grad_rot_0_local_t(2)) * Iy_avg +
                        1.5 * Utility::pow<2>(_grad_rot_0_local_t(1)) * Iz_avg +
                        0.5 * Utility::pow<2>(_grad_disp_0_local_t(1)) +
                        0.5 * Utility::pow<2>(_grad_disp_0_local_t(2)) +
                        0.5 * Utility::pow<2>(_grad_rot_0_local_t(0)) * Ix_avg;
    k1_large_11(1, 0) = 0.5 * _grad_disp_0_local_t(0) * _grad_disp_0_local_t(1) -
                        1.0 / 3.0 * _grad_rot_0_local_t(0) * _grad_rot_0_local_t(1) * Iz_avg;
    k1_large_11(2, 0) = 0.5 * _grad_disp_0_local_t(0) * _grad_disp_0_local_t(2) -
                        1.0 / 3.0 * _grad_rot_0_local_t(0) * _grad_rot_0_local_t(2) * Iy_avg;

    // row 2
    k1_large_11(0, 1) = k1_large_11(1, 0);
    k1_large_11(1, 1) = Utility::pow<2>(_grad_disp_0_local_t(1)) +
                        1.5 * Utility::pow<2>(_grad_rot_0_local_t(0)) * Iz_avg +
                        0.5 * Utility::pow<2>(_grad_disp_0_local_t(0)) +
                        0.5 * Utility::pow<2>(_grad_disp_0_local_t(2)) +
                        0.5 * Utility::pow<2>(_grad_rot_0_local_t(2)) * Iy_avg +
                        0.5 * Utility::pow<2>(_grad_rot_0_local_t(1)) * Iz_avg +
                        0.5 * Utility::pow<2>(_grad_rot_0_local_t(0)) * Iy_avg;
    k1_large_11(2, 1) = 0.5 * _grad_disp_0_local_t(1) * _grad_disp_0_local_t(2);

    // row 3
    k1_large_11(0, 2) = k1_large_11(2, 0);
    k1_large_11(1, 2) = k1_large_11(2, 1);
    k1_large_11(2, 2) = Utility::pow<2>(_grad_disp_0_local_t(2)) +
                        1.5 * Utility::pow<2>(_grad_rot_0_local_t(0)) * Iy_avg +
                        0.5 * Utility::pow<2>(_grad_disp_0_local_t(0)) +
                        0.5 * Utility::pow<2>(_grad_disp_0_local_t(1)) +
                        0.5 * Utility::pow<2>(_grad_rot_0_local_t(0)) * Iz_avg +
                        0.5 * Utility::pow<2>(_grad_rot_0_local_t(2)) * Iy_avg +
                        0.5 * Utility::pow<2>(_grad_rot_0_local_t(2)) * Iz_avg;

    k1_large_11 *= 1.0 / 4.0 / Utility::pow<2>(_original_length[0]);

    RankTwoTensor k1_large_21;
    // row 1
    k1_large_21(0, 0) = 0.5 * _grad_disp_0_local_t(0) * _grad_rot_0_local_t(0) * (Ix_avg)-1.0 /
                            3.0 * _grad_disp_0_local_t(1) * _grad_rot_0_local_t(1) * Iz_avg -
                        1.0 / 3.0 * _grad_disp_0_local_t(2) * _grad_rot_0_local_t(2);
    k1_large_21(1, 0) = 1.5 * _grad_disp_0_local_t(0) * _grad_rot_0_local_t(1) * Iz_avg -
                        1.0 / 3.0 * _grad_disp_0_local_t(1) * _grad_rot_0_local_t(0) * Iz_avg;
    k1_large_21(2, 0) = 1.5 * _grad_disp_0_local_t(0) * _grad_rot_0_local_t(2) * Iy_avg -
                        1.0 / 3.0 * _grad_disp_0_local_t(2) * _grad_rot_0_local_t(0) * Iy_avg;

    // row 2
    k1_large_21(0, 1) = k1_large_21(1, 0);
    k1_large_21(1, 1) = 0.5 * _grad_disp_0_local_t(1) * _grad_rot_0_local_t(1) * Iz_avg -
                        1.0 / 3.0 * _grad_disp_0_local_t(0) * _grad_rot_0_local_t(0) * Iz_avg;
    k1_large_21(2, 1) = 0.5 * _grad_disp_0_local_t(1) * _grad_rot_0_local_t(2) * Iy_avg;

    // row 3
    k1_large_21(0, 2) = k1_large_21(2, 0);
    k1_large_21(1, 2) = k1_large_21(2, 1);
    k1_large_21(2, 2) = 0.5 * _grad_disp_0_local_t(2) * _grad_rot_0_local_t(2) * Iy_avg -
                        1.0 / 3.0 * _grad_disp_0_local_t(0) * _grad_rot_0_local_t(0) * Iy_avg;
    k1_large_21 *= 1.0 / 4.0 / Utility::pow<2>(_original_length[0]);

    RankTwoTensor k1_large_22;
    // row 1
    k1_large_22(0, 0) = Utility::pow<2>(_grad_rot_0_local_t(0)) * Utility::pow<2>(Ix_avg) +
                        1.5 * Utility::pow<2>(_grad_disp_0_local_t(1)) * Iz_avg +
                        1.5 * Utility::pow<2>(_grad_disp_0_local_t(2)) * Iy_avg +
                        0.5 * Utility::pow<2>(_grad_disp_0_local_t(0)) * Ix_avg +
                        0.5 * Utility::pow<2>(_grad_disp_0_local_t(2)) * Iz_avg +
                        0.5 * Utility::pow<2>(_grad_disp_0_local_t(1)) * Iy_avg +
                        0.5 * Utility::pow<2>(_grad_rot_0_local_t(2)) * Iy_avg * Ix_avg +
                        0.5 * Utility::pow<2>(_grad_rot_0_local_t(1)) * Iz_avg * Ix_avg;
    k1_large_22(1, 0) = 0.5 * _grad_rot_0_local_t(0) * _grad_rot_0_local_t(1) * Iz_avg * Ix_avg -
                        1.0 / 3.0 * _grad_disp_0_local_t(0) * _grad_disp_0_local_t(1) * Iz_avg;
    k1_large_22(2, 0) = 0.5 * _grad_rot_0_local_t(0) * _grad_rot_0_local_t(2) * Iy_avg * Ix_avg -
                        1.0 / 3.0 * _grad_disp_0_local_t(0) * _grad_disp_0_local_t(2) * Iy_avg;

    // row 2
    k1_large_22(0, 1) = k1_large_22(1, 0);
    k1_large_22(1, 1) = Utility::pow<2>(_grad_rot_0_local_t(1)) * Iz_avg * Iz_avg +
                        1.5 * Utility::pow<2>(_grad_disp_0_local_t(0)) * Iz_avg +
                        1.5 * Utility::pow<2>(_grad_rot_0_local_t(2)) * Iy_avg * Iz_avg +
                        0.5 * Utility::pow<2>(_grad_disp_0_local_t(1)) * Iz_avg +
                        0.5 * Utility::pow<2>(_grad_disp_0_local_t(2)) * Iz_avg +
                        0.5 * Utility::pow<2>(_grad_rot_0_local_t(0)) * Iz_avg * Ix_avg;
    k1_large_22(2, 1) = 1.5 * _grad_rot_0_local_t(1) * _grad_rot_0_local_t(2) * Iy_avg * Iz_avg;

    // row 3
    k1_large_22(0, 2) = k1_large_22(2, 0);
    k1_large_22(1, 2) = k1_large_22(2, 1);
    k1_large_22(2, 2) = Utility::pow<2>(_grad_rot_0_local_t(2)) * Iy_avg * Iy_avg +
                        1.5 * Utility::pow<2>(_grad_disp_0_local_t(0)) * Iy_avg +
                        1.5 * Utility::pow<2>(_grad_rot_0_local_t(1)) * Iy_avg * Iz_avg +
                        0.5 * Utility::pow<2>(_grad_disp_0_local_t(1)) * Iy_avg +
                        0.5 * Utility::pow<2>(_grad_disp_0_local_t(2)) * Iy_avg +
                        0.5 * Utility::pow<2>(_grad_rot_0_local_t(0)) * Iz_avg * Ix_avg;

    k1_large_22 *= 1.0 / 4.0 / Utility::pow<2>(_original_length[0]);

    // k2_large and k3_large are contributions from tau_xy * d(gamma_xy) and tau_xz * d(gamma_xz)
    // k2_large for node 1 is negative of that for node 0
    RankTwoTensor k2_large_11;
    // col 1
    k2_large_11(0, 0) =
        0.25 * Utility::pow<2>(_avg_rot_local_t(2)) + 0.25 * Utility::pow<2>(_avg_rot_local_t(1));
    k2_large_11(1, 0) = -1.0 / 6.0 * _avg_rot_local_t(0) * _avg_rot_local_t(1);
    k2_large_11(2, 0) = -1.0 / 6.0 * _avg_rot_local_t(0) * _avg_rot_local_t(2);

    // col 2
    k2_large_11(0, 1) = k2_large_11(1, 0);
    k2_large_11(1, 1) = 0.25 * _avg_rot_local_t(0);

    // col 3
    k2_large_11(0, 2) = k2_large_11(2, 0);
    k2_large_11(2, 2) = 0.25 * Utility::pow<2>(_avg_rot_local_t(0));

    k2_large_11 *= 1.0 / 4.0 / Utility::pow<2>(_original_length[0]);

    RankTwoTensor k2_large_22;
    // col1
    k2_large_22(0, 0) = 0.25 * Utility::pow<2>(_avg_rot_local_t(0)) * Ix_avg;
    k2_large_22(1, 0) = 1.0 / 6.0 * _avg_rot_local_t(0) * _avg_rot_local_t(1) * Iz_avg;
    k2_large_22(2, 0) = 1.0 / 6.0 * _avg_rot_local_t(0) * _avg_rot_local_t(2) * Iy_avg;

    // col2
    k2_large_22(0, 1) = k2_large_22(1, 0);
    k2_large_22(1, 1) = 0.25 * Utility::pow<2>(_avg_rot_local_t(2)) * Iz_avg +
                        0.25 * Utility::pow<2>(_avg_rot_local_t(1)) * Iz_avg;

    // col3
    k2_large_22(0, 2) = k2_large_22(2, 0);
    k2_large_22(2, 2) = 0.25 * Utility::pow<2>(_avg_rot_local_t(2)) * Iy_avg +
                        0.25 * Utility::pow<2>(_avg_rot_local_t(1)) * Iy_avg;

    k2_large_22 *= 1.0 / 4.0 / Utility::pow<2>(_original_length[0]);

    // k3_large for node 1 is same as that for node 0
    RankTwoTensor k3_large_22;
    // col1
    k3_large_22(0, 0) = 0.25 * Utility::pow<2>(_grad_disp_0_local_t(2)) +
                        0.25 * _grad_rot_0_local_t(0) * Ix_avg +
                        0.25 * Utility::pow<2>(_grad_disp_0_local_t(1));
    k3_large_22(1, 0) = -1.0 / 6.0 * _grad_disp_0_local_t(0) * _grad_disp_0_local_t(1) +
                        1.0 / 6.0 * _grad_rot_0_local_t(0) * _grad_rot_0_local_t(1) * Iz_avg;
    k3_large_22(2, 0) = -1.0 / 6.0 * _grad_disp_0_local_t(0) * _grad_disp_0_local_t(2) +
                        1.0 / 6.0 * _grad_rot_0_local_t(0) * _grad_rot_0_local_t(2) * Iy_avg;

    // col2
    k3_large_22(0, 1) = k3_large_22(1, 0);
    k3_large_22(2, 2) = 0.25 * Utility::pow<2>(_grad_disp_0_local_t(0)) +
                        0.25 * _grad_rot_0_local_t(2) * Iy_avg +
                        0.25 * _grad_rot_0_local_t(1) * Iz_avg;

    // col3
    k3_large_22(0, 2) = k3_large_22(2, 0);
    k3_large_22(2, 2) = 0.25 * Utility::pow<2>(_grad_disp_0_local_t(0)) +
                        0.25 * _grad_rot_0_local_t(2) * Iy_avg +
                        0.25 * _grad_rot_0_local_t(1) * Iz_avg;

    k3_large_22 *= 1.0 / 16.0;

    RankTwoTensor k3_large_21;
    // col1
    k3_large_21(0, 0) = -1.0 / 6.0 *
                        (_grad_disp_0_local_t(2) * _avg_rot_local_t(2) +
                         _grad_disp_0_local_t(1) * _avg_rot_local_t(1));
    k3_large_21(1, 0) = 0.25 * _grad_disp_0_local_t(0) * _avg_rot_local_t(1) -
                        1.0 / 6.0 * _grad_disp_0_local_t(1) * _avg_rot_local_t(0);
    k3_large_21(2, 0) = 0.25 * _grad_disp_0_local_t(0) * _avg_rot_local_t(2) -
                        1.0 / 6.0 * _grad_disp_0_local_t(2) * _avg_rot_local_t(0);

    // col2
    k3_large_21(0, 1) = 0.25 * _grad_disp_0_local_t(1) * _avg_rot_local_t(0) -
                        1.0 / 6.0 * _grad_disp_0_local_t(0) * _avg_rot_local_t(1);
    k3_large_21(1, 1) = -1.0 / 6.0 * _grad_disp_0_local_t(0) * _avg_rot_local_t(0);

    // col3
    k3_large_21(0, 2) = 0.25 * _grad_disp_0_local_t(2) * _avg_rot_local_t(0) -
                        1.0 / 6.0 * _grad_disp_0_local_t(0) * _avg_rot_local_t(2);
    k3_large_21(2, 2) = -1.0 / 6.0 * _grad_disp_0_local_t(0) * _avg_rot_local_t(0);

    k3_large_21 *= 1.0 / 8.0 / _original_length[0];

    RankTwoTensor k4_large_22;
    // col 1
    k4_large_22(0, 0) = 0.25 * _grad_rot_0_local_t(0) * _avg_rot_local_t(0) * Ix_avg +
                        1.0 / 6.0 * _grad_rot_0_local_t(2) * _avg_rot_local_t(2) * Iy_avg +
                        1.0 / 6.0 * _grad_rot_0_local_t(1) * _avg_rot_local_t(1) * Iz_avg;
    k4_large_22(1, 0) = 1.0 / 6.0 * _grad_rot_0_local_t(1) * _avg_rot_local_t(0) * Iz_avg;
    k4_large_22(2, 0) = 1.0 / 6.0 * _grad_rot_0_local_t(2) * _avg_rot_local_t(0) * Iy_avg;

    // col2
    k4_large_22(0, 1) = 1.0 / 6.0 * _grad_rot_0_local_t(0) * _avg_rot_local_t(1) * Iz_avg;
    k4_large_22(1, 1) = 0.25 * _grad_rot_0_local_t(1) * _avg_rot_local_t(1) * Iz_avg +
                        1.0 / 6.0 * _grad_rot_0_local_t(0) * _avg_rot_local_t(0) * Iz_avg;
    k4_large_22(2, 1) = 0.25 * _grad_rot_0_local_t(1) * _avg_rot_local_t(2) * Iz_avg;

    // col 3
    k4_large_22(0, 2) = 1.0 / 6.0 * _grad_rot_0_local_t(0) * _avg_rot_local_t(2) * Iy_avg;
    k4_large_22(1, 2) = 0.25 * _grad_rot_0_local_t(2) * _avg_rot_local_t(1) * Iy_avg;
    k4_large_22(2, 2) = 0.25 * _grad_rot_0_local_t(2) * _avg_rot_local_t(2) * Iy_avg +
                        1.0 / 6.0 * _grad_rot_0_local_t(0) * _avg_rot_local_t(0) * Iy_avg;

    k3_large_22 += 1.0 / 8.0 / _original_length[0] * (k4_large_22 + k4_large_22.transpose());

    // Assembling final matrix
    _K11[0] += _total_rotation[0].transpose() * (k1_large_11 + k2_large_11) * _total_rotation[0];
    _K22[0] += _total_rotation[0].transpose() * (k1_large_22 + k2_large_22 + k3_large_22) *
               _total_rotation[0];
    _K21[0] += _total_rotation[0].transpose() * (k1_large_21 + k3_large_21) * _total_rotation[0];
    _K21_cross[0] +=
        _total_rotation[0].transpose() * (-k1_large_21 + k3_large_21) * _total_rotation[0];
    _K22_cross[0] += _total_rotation[0].transpose() * (-k1_large_22 - k2_large_22 + k3_large_22) *
                     _total_rotation[0];

  }
*/
}

void
ComputeIncrementalTrussStrain::computeRotation()
{
  _total_rotation[0] = _original_local_config;
}
