//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DiffusionHDGKernel.h"
#include "MooseVariableFE.h"
#include "MooseVariableScalar.h"
#include "Function.h"
#include "SystemBase.h"
#include "MooseMesh.h"
#include "MooseObject.h"
#include "MaterialPropertyInterface.h"
#include "NonlinearThread.h"

using namespace libMesh;

registerMooseObject("MooseApp", DiffusionHDGKernel);

InputParameters
DiffusionHDGKernel::diffusionParams()
{
  auto params = emptyInputParameters();
  params.addRequiredParam<NonlinearVariableName>(
      "grad_u", "The gradient of the diffusing specie concentration");
  params.addRequiredParam<NonlinearVariableName>(
      "face_u", "The concentration of the diffusing specie on faces");
  params.addRequiredParam<MaterialPropertyName>("diffusivity", "The diffusivity");
  params.addParam<Real>("tau",
                        1,
                        "The stabilization coefficient required for discontinuous Galerkin "
                        "schemes. This may be set to 0 for a mixed method with Raviart-Thomas.");
  params.addParam<MooseFunctorName>("source", 0, "Source for the diffusing species");
  return params;
}

InputParameters
DiffusionHDGKernel::validParams()
{
  auto params = Kernel::validParams();
  params += DiffusionHDGKernel::diffusionParams();
  params.renameParam("variable", "u", "The diffusing specie concentration");
  return params;
}

DiffusionHDGKernel::DiffusionHDGKernel(const InputParameters & params)
  : Kernel(params),
    NonADFunctorInterface(this),
    constructDiffusion(),
    _source(getFunctor<Real>("source")),
    _qrule_face(_assembly.qRuleFace()),
    _q_point_face(_assembly.qPointsFace()),
    _JxW_face(_assembly.JxWFace()),
    _coord_face(_assembly.coordTransformation()),
    _normals(_assembly.normals()),
    _current_side(_assembly.side())
{
  addMooseVariableDependency(&const_cast<MooseVariableFE<Real> &>(_u_var));
  addMooseVariableDependency(&const_cast<MooseVariableFE<RealVectorValue> &>(_grad_u_var));
  addMooseVariableDependency(&const_cast<MooseVariableFE<Real> &>(_u_face_var));
}

void
DiffusionHDGKernel::checkCoupling(const FEProblemBase & fe_problem,
                                  const unsigned int nl_sys_num,
                                  const MooseObject & diffusion_obj)
{
  if (fe_problem.coupling() == Moose::COUPLING_FULL)
    return;
  else if (fe_problem.coupling() == Moose::COUPLING_CUSTOM)
  {
    const auto * const cm = fe_problem.couplingMatrix(nl_sys_num);
    for (const auto i : make_range(cm->size()))
      for (const auto j : make_range(cm->size()))
        if ((*cm)(i, j) != true)
          goto error;

    return;
  }

error:
  diffusion_obj.mooseError(
      "This class encodes the full Jacobian regardless of user input file specification, "
      "so please request full coupling in your Preconditioning block for consistency");
}
void
DiffusionHDGKernel::initialSetup()
{
  // This check must occur after FEProblemBase::init()
  checkCoupling(_fe_problem, _sys.number(), *this);
}

void
DiffusionHDGKernel::computeResidual()
{
  _vector_re.resize(_qu_dof_indices.size());
  _scalar_re.resize(_u_dof_indices.size());
  _lm_re.resize(_lm_u_dof_indices.size());

  // qu and u
  vectorVolumeResidual(_qu_sol, _u_sol, _JxW, *_qrule, _vector_phi, _div_vector_phi, _vector_re);
  scalarVolumeResidual(_qu_sol,
                       _source,
                       _JxW,
                       *_qrule,
                       _current_elem,
                       _q_point,
                       _grad_scalar_phi,
                       _scalar_phi,
                       _diff,
                       *this,
                       _scalar_re);

  for (const auto side : _current_elem->side_index_range())
    if (_neigh = _current_elem->neighbor_ptr(side);
        _neigh && this->hasBlocks(_neigh->subdomain_id()))
    {
      NonlinearThread::prepareFace(
          _fe_problem, _tid, _current_elem, side, Moose::INVALID_BOUNDARY_ID);
      mooseAssert(_current_side == side, "The sides should be the same");
      // qu, u, lm_u
      vectorFaceResidual(
          _lm_u_sol, _JxW_face, *_qrule_face, _normals, _vector_phi_face, _vector_re);
      scalarFaceResidual(_qu_sol,
                         _u_sol,
                         _lm_u_sol,
                         _JxW_face,
                         *_qrule_face,
                         _normals,
                         _scalar_phi_face,
                         _diff,
                         _tau,
                         _scalar_re);
      lmFaceResidual(_qu_sol,
                     _u_sol,
                     _lm_u_sol,
                     _JxW_face,
                     *_qrule_face,
                     _normals,
                     _lm_phi_face,
                     _diff,
                     _tau,
                     _lm_re);
    }

  addResiduals(_assembly, _vector_re, _qu_dof_indices, _grad_u_var.scalingFactor());
  addResiduals(_assembly, _scalar_re, _u_dof_indices, _u_var.scalingFactor());
  addResiduals(_assembly, _lm_re, _lm_u_dof_indices, _u_face_var.scalingFactor());
}

void
DiffusionHDGKernel::computeJacobian()
{
  _vector_vector_jac.resize(_qu_dof_indices.size(), _qu_dof_indices.size());
  _vector_scalar_jac.resize(_qu_dof_indices.size(), _u_dof_indices.size());
  _scalar_vector_jac.resize(_u_dof_indices.size(), _qu_dof_indices.size());
  _scalar_scalar_jac.resize(_u_dof_indices.size(), _u_dof_indices.size());
  _scalar_lm_jac.resize(_u_dof_indices.size(), _lm_u_dof_indices.size());
  _lm_scalar_jac.resize(_lm_u_dof_indices.size(), _u_dof_indices.size());
  _lm_lm_jac.resize(_lm_u_dof_indices.size(), _lm_u_dof_indices.size());
  _vector_lm_jac.resize(_qu_dof_indices.size(), _lm_u_dof_indices.size());
  _lm_vector_jac.resize(_lm_u_dof_indices.size(), _qu_dof_indices.size());

  // qu and u
  vectorVolumeJacobian(_JxW,
                       *_qrule,
                       _vector_phi,
                       _div_vector_phi,
                       _scalar_phi,
                       _vector_vector_jac,
                       _vector_scalar_jac);
  scalarVolumeJacobian(_JxW, *_qrule, _grad_scalar_phi, _vector_phi, _diff, _scalar_vector_jac);

  for (const auto side : _current_elem->side_index_range())
    if (_neigh = _current_elem->neighbor_ptr(side);
        _neigh && this->hasBlocks(_neigh->subdomain_id()))
    {
      NonlinearThread::prepareFace(
          _fe_problem, _tid, _current_elem, side, Moose::INVALID_BOUNDARY_ID);
      mooseAssert(_current_side == side, "The sides should be the same");
      // qu, u, lm_u
      vectorFaceJacobian(
          _JxW_face, *_qrule_face, _normals, _vector_phi_face, _lm_phi_face, _vector_lm_jac);
      scalarFaceJacobian(_JxW_face,
                         *_qrule_face,
                         _normals,
                         _scalar_phi_face,
                         _vector_phi_face,
                         _lm_phi_face,
                         _diff,
                         _tau,
                         _scalar_vector_jac,
                         _scalar_scalar_jac,
                         _scalar_lm_jac);
      lmFaceJacobian(_JxW_face,
                     *_qrule_face,
                     _normals,
                     _lm_phi_face,
                     _vector_phi_face,
                     _scalar_phi_face,
                     _diff,
                     _tau,
                     _lm_vector_jac,
                     _lm_scalar_jac,
                     _lm_lm_jac);
    }

  addJacobian(
      _assembly, _vector_vector_jac, _qu_dof_indices, _qu_dof_indices, _grad_u_var.scalingFactor());
  addJacobian(
      _assembly, _vector_scalar_jac, _qu_dof_indices, _u_dof_indices, _grad_u_var.scalingFactor());
  addJacobian(
      _assembly, _scalar_vector_jac, _u_dof_indices, _qu_dof_indices, _u_var.scalingFactor());
  addJacobian(
      _assembly, _scalar_scalar_jac, _u_dof_indices, _u_dof_indices, _u_var.scalingFactor());
  addJacobian(_assembly, _scalar_lm_jac, _u_dof_indices, _lm_u_dof_indices, _u_var.scalingFactor());
  addJacobian(
      _assembly, _lm_scalar_jac, _lm_u_dof_indices, _u_dof_indices, _u_face_var.scalingFactor());
  addJacobian(
      _assembly, _lm_lm_jac, _lm_u_dof_indices, _lm_u_dof_indices, _u_face_var.scalingFactor());
  addJacobian(
      _assembly, _vector_lm_jac, _qu_dof_indices, _lm_u_dof_indices, _grad_u_var.scalingFactor());
  addJacobian(
      _assembly, _lm_vector_jac, _lm_u_dof_indices, _qu_dof_indices, _u_face_var.scalingFactor());
}

void
DiffusionHDGKernel::vectorVolumeResidual(
    const MooseArray<Gradient> & vector_sol,
    const MooseArray<Number> & scalar_sol,
    const MooseArray<Real> & JxW,
    const QBase & qrule,
    const MooseArray<std::vector<RealVectorValue>> & vector_phi,
    const MooseArray<std::vector<Real>> & div_vector_phi,
    DenseVector<Number> & vector_re)
{
  for (const auto i : index_range(vector_re))
    for (const auto qp : make_range(qrule.n_points()))
    {
      // Vector equation dependence on vector dofs
      vector_re(i) += JxW[qp] * (vector_phi[i][qp] * vector_sol[qp]);

      // Vector equation dependence on scalar dofs
      vector_re(i) += JxW[qp] * (div_vector_phi[i][qp] * scalar_sol[qp]);
    }
}

void
DiffusionHDGKernel::vectorVolumeJacobian(
    const MooseArray<Real> & JxW,
    const QBase & qrule,
    const MooseArray<std::vector<RealVectorValue>> & vector_phi,
    const MooseArray<std::vector<Real>> & div_vector_phi,
    const MooseArray<std::vector<Real>> & scalar_phi,
    DenseMatrix<Number> & vector_vector_jac,
    DenseMatrix<Number> & vector_scalar_jac)
{
  for (const auto i : make_range(vector_vector_jac.m()))
    for (const auto qp : make_range(qrule.n_points()))
    {
      // Vector equation dependence on vector dofs
      for (const auto j : make_range(vector_vector_jac.n()))
        vector_vector_jac(i, j) += JxW[qp] * (vector_phi[i][qp] * vector_phi[j][qp]);

      // Vector equation dependence on scalar dofs
      for (const auto j : make_range(vector_scalar_jac.n()))
        vector_scalar_jac(i, j) += JxW[qp] * (div_vector_phi[i][qp] * scalar_phi[j][qp]);
    }
}

void
DiffusionHDGKernel::scalarVolumeResidual(
    const MooseArray<Gradient> & vector_field,
    const Moose::Functor<Real> & source,
    const MooseArray<Real> & JxW,
    const QBase & qrule,
    const Elem * const current_elem,
    const MooseArray<Point> & q_point,
    const MooseArray<std::vector<RealVectorValue>> & grad_scalar_phi,
    const MooseArray<std::vector<Real>> & scalar_phi,
    const MaterialProperty<Real> & diff,
    const TransientInterface & ti,
    DenseVector<Number> & scalar_re)
{
  for (const auto qp : make_range(qrule.n_points()))
  {
    // Evaluate source
    const auto f =
        source(Moose::ElemQpArg{current_elem, qp, &qrule, q_point[qp]}, ti.determineState());

    for (const auto i : index_range(scalar_re))
    {
      scalar_re(i) += JxW[qp] * (grad_scalar_phi[i][qp] * diff[qp] * vector_field[qp]);

      // Scalar equation RHS
      scalar_re(i) -= JxW[qp] * scalar_phi[i][qp] * f;
    }
  }
}

void
DiffusionHDGKernel::scalarVolumeJacobian(
    const MooseArray<Real> & JxW,
    const QBase & qrule,
    const MooseArray<std::vector<RealVectorValue>> & grad_scalar_phi,
    const MooseArray<std::vector<RealVectorValue>> & vector_phi,
    const MaterialProperty<Real> & diff,
    DenseMatrix<Number> & scalar_vector_jac)
{
  for (const auto i : make_range(scalar_vector_jac.m()))
    // Scalar equation dependence on vector dofs
    for (const auto j : make_range(scalar_vector_jac.n()))
      for (const auto qp : make_range(qrule.n_points()))
        scalar_vector_jac(i, j) +=
            JxW[qp] * diff[qp] * (grad_scalar_phi[i][qp] * vector_phi[j][qp]);
}

void
DiffusionHDGKernel::vectorFaceResidual(
    const MooseArray<Number> & lm_sol,
    const MooseArray<Real> & JxW_face,
    const QBase & qrule_face,
    const MooseArray<Point> & normals,
    const MooseArray<std::vector<RealVectorValue>> & vector_phi_face,
    DenseVector<Number> & vector_re)
{
  // Vector equation dependence on LM dofs
  for (const auto i : index_range(vector_re))
    for (const auto qp : make_range(qrule_face.n_points()))
      vector_re(i) -= JxW_face[qp] * (vector_phi_face[i][qp] * normals[qp]) * lm_sol[qp];
}

void
DiffusionHDGKernel::vectorFaceJacobian(
    const MooseArray<Real> & JxW_face,
    const QBase & qrule_face,
    const MooseArray<Point> & normals,
    const MooseArray<std::vector<RealVectorValue>> & vector_phi_face,
    const MooseArray<std::vector<Real>> & lm_phi_face,
    DenseMatrix<Number> & vector_lm_jac)
{
  // Vector equation dependence on LM dofs
  for (const auto i : make_range(vector_lm_jac.m()))
    for (const auto j : make_range(vector_lm_jac.n()))
      for (const auto qp : make_range(qrule_face.n_points()))
        vector_lm_jac(i, j) -=
            JxW_face[qp] * (vector_phi_face[i][qp] * normals[qp]) * lm_phi_face[j][qp];
}

void
DiffusionHDGKernel::scalarFaceResidual(const MooseArray<Gradient> & vector_sol,
                                       const MooseArray<Number> & scalar_sol,
                                       const MooseArray<Number> & lm_sol,
                                       const MooseArray<Real> & JxW_face,
                                       const QBase & qrule_face,
                                       const MooseArray<Point> & normals,
                                       const MooseArray<std::vector<Real>> & scalar_phi_face,
                                       const MaterialProperty<Real> & diff,
                                       const Real tau,
                                       DenseVector<Number> & scalar_re)
{
  for (const auto i : index_range(scalar_re))
    for (const auto qp : make_range(qrule_face.n_points()))
    {
      // vector
      scalar_re(i) -=
          JxW_face[qp] * diff[qp] * scalar_phi_face[i][qp] * (vector_sol[qp] * normals[qp]);

      // scalar from stabilization term
      scalar_re(i) +=
          JxW_face[qp] * scalar_phi_face[i][qp] * tau * scalar_sol[qp] * normals[qp] * normals[qp];

      // lm from stabilization term
      scalar_re(i) -=
          JxW_face[qp] * scalar_phi_face[i][qp] * tau * lm_sol[qp] * normals[qp] * normals[qp];
    }
}

void
DiffusionHDGKernel::scalarFaceJacobian(
    const MooseArray<Real> & JxW_face,
    const QBase & qrule_face,
    const MooseArray<Point> & normals,
    const MooseArray<std::vector<Real>> & scalar_phi_face,
    const MooseArray<std::vector<RealVectorValue>> & vector_phi_face,
    const MooseArray<std::vector<Real>> & lm_phi_face,
    const MaterialProperty<Real> & diff,
    const Real tau,
    DenseMatrix<Number> & scalar_vector_jac,
    DenseMatrix<Number> & scalar_scalar_jac,
    DenseMatrix<Number> & scalar_lm_jac)
{
  for (const auto i : make_range(scalar_vector_jac.m()))
    for (const auto qp : make_range(qrule_face.n_points()))
    {
      for (const auto j : make_range(scalar_vector_jac.n()))
        scalar_vector_jac(i, j) -= JxW_face[qp] * diff[qp] * scalar_phi_face[i][qp] *
                                   (vector_phi_face[j][qp] * normals[qp]);

      for (const auto j : make_range(scalar_scalar_jac.n()))
        scalar_scalar_jac(i, j) += JxW_face[qp] * scalar_phi_face[i][qp] * tau *
                                   scalar_phi_face[j][qp] * normals[qp] * normals[qp];

      for (const auto j : make_range(scalar_lm_jac.n()))
        // from stabilization term
        scalar_lm_jac(i, j) -= JxW_face[qp] * scalar_phi_face[i][qp] * tau * lm_phi_face[j][qp] *
                               normals[qp] * normals[qp];
    }
}

void
DiffusionHDGKernel::lmFaceResidual(const MooseArray<Gradient> & vector_sol,
                                   const MooseArray<Number> & scalar_sol,
                                   const MooseArray<Number> & lm_sol,
                                   const MooseArray<Real> & JxW_face,
                                   const QBase & qrule_face,
                                   const MooseArray<Point> & normals,
                                   const MooseArray<std::vector<Real>> & lm_phi_face,
                                   const MaterialProperty<Real> & diff,
                                   const Real tau,
                                   DenseVector<Number> & lm_re)
{
  for (const auto i : index_range(lm_re))
    for (const auto qp : make_range(qrule_face.n_points()))
    {
      // vector
      lm_re(i) -= JxW_face[qp] * diff[qp] * lm_phi_face[i][qp] * (vector_sol[qp] * normals[qp]);

      // scalar from stabilization term
      lm_re(i) +=
          JxW_face[qp] * lm_phi_face[i][qp] * tau * scalar_sol[qp] * normals[qp] * normals[qp];

      // lm from stabilization term
      lm_re(i) -= JxW_face[qp] * lm_phi_face[i][qp] * tau * lm_sol[qp] * normals[qp] * normals[qp];
    }
}

void
DiffusionHDGKernel::lmFaceJacobian(const MooseArray<Real> & JxW_face,
                                   const QBase & qrule_face,
                                   const MooseArray<Point> & normals,
                                   const MooseArray<std::vector<Real>> & lm_phi_face,
                                   const MooseArray<std::vector<RealVectorValue>> & vector_phi_face,
                                   const MooseArray<std::vector<Real>> & scalar_phi_face,
                                   const MaterialProperty<Real> & diff,
                                   const Real tau,
                                   DenseMatrix<Number> & lm_vec_jac,
                                   DenseMatrix<Number> & lm_scalar_jac,
                                   DenseMatrix<Number> & lm_lm_jac)
{
  for (const auto i : make_range(lm_vec_jac.m()))
    for (const auto qp : make_range(qrule_face.n_points()))
    {
      for (const auto j : make_range(lm_vec_jac.n()))
        lm_vec_jac(i, j) -=
            JxW_face[qp] * diff[qp] * lm_phi_face[i][qp] * (vector_phi_face[j][qp] * normals[qp]);

      for (const auto j : make_range(lm_scalar_jac.n()))
        lm_scalar_jac(i, j) += JxW_face[qp] * lm_phi_face[i][qp] * tau * scalar_phi_face[j][qp] *
                               normals[qp] * normals[qp];

      for (const auto j : make_range(lm_lm_jac.n()))
        // from stabilization term
        lm_lm_jac(i, j) -= JxW_face[qp] * lm_phi_face[i][qp] * tau * lm_phi_face[j][qp] *
                           normals[qp] * normals[qp];
    }
}

void
DiffusionHDGKernel::jacobianSetup()
{
  _my_elem = nullptr;
}

void
DiffusionHDGKernel::computeOffDiagJacobian(const unsigned int)
{
  if (_my_elem != _current_elem)
  {
    computeJacobian();
    _my_elem = _current_elem;
  }
}

std::set<std::string>
DiffusionHDGKernel::additionalVariablesCovered()
{
  return {_u_var.name(), _grad_u_var.name(), _u_face_var.name()};
}
