//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef NEML2_ENABLED

// MOOSE includes
#include "TorchStressDivergence.h"
#include "TorchFEMUtils.h"

registerMooseObject("SolidMechanicsApp", TorchStressDivergence);

InputParameters
TorchStressDivergence::validParams()
{
  InputParameters params = TorchPostKernel::validParams();
  params.addClassDescription(
      "This user object calculates the stress divergence for a given set of displacement "
      "variables and assembles the contribution into the residual vector.");
  params.addRequiredParam<std::vector<NonlinearVariableName>>(
      "displacements",
      "The displacements variables whose function space will be used to define the test "
      "functions.");
  params.addRequiredParam<std::string>("stress",
                                       "The name of the NEML2 variable to use as the stress.");
  params.addRequiredParam<std::string>("residual", "The tag for the residual");
  return params;
}

TorchStressDivergence::TorchStressDivergence(const InputParameters & parameters)
  : TorchPostKernel(parameters),
    _residual(dynamic_cast<libMesh::PetscVector<Real> *>(&_sys.system().add_vector(
        getParam<std::string>("residual"), false, libMesh::ParallelType::GHOSTED))),
    _stress(_constitutive.getOutput(getParam<std::string>("stress"))),
    _disp_vars(getParam<std::vector<NonlinearVariableName>>("displacements")),
    _ndisp(_disp_vars.size())
{
  mooseAssert(_residual, "Failed to cast residual to PetscVector<Real>");

  if (_disp_vars.size() < 1 || _disp_vars.size() > 3)
    mooseError("TorchStressDivergence requires 1 to 3 displacement variables, got ",
               _disp_vars.size());

  _grad_test_x = &_fe.getPhiGradient(_disp_vars[0]);
  _grad_test_y = _ndisp >= 2 ? &_fe.getPhiGradient(_disp_vars[1]) : nullptr;
  _grad_test_z = _ndisp >= 3 ? &_fe.getPhiGradient(_disp_vars[2]) : nullptr;
}

void
TorchStressDivergence::forward()
{
  if (!_constitutive.outputReady())
    return;

  torch::InferenceMode guard;
  using at::indexing::Slice;

  // test function gradients, each (nelem, ndofe, nqp, 3)
  auto dphix = *_grad_test_x;
  auto dphiy = _ndisp >= 2 ? *_grad_test_y : at::zeros_like(dphix);
  auto dphiz = _ndisp >= 3 ? *_grad_test_z : at::zeros_like(dphix);
  // stack along the displacement-component axis: dphi[e, a, q, i, j] = d phi_a / d x_j for
  // component i, shape (nelem, ndofe, nqp, 3, 3)
  auto dphi = at::stack({dphix, dphiy, dphiz}, 3);
  // full stress sigma[e, q, i, j], broadcast over the dof axis: (nelem, 1, nqp, 3, 3)
  auto stress = TorchFEM::mandelToFull(_stress).unsqueeze(1);

  // weak form: re_qp[e, a, q, i] = sum_j dphi_ij * sigma_ij, shape (nelem, ndofe, nqp, 3)
  auto re_qp = (dphi * stress).sum(-1);

  // element integration over quadrature points
  auto JxWxT = _assembly.JxWxT().unsqueeze(1).unsqueeze(-1); // (nelem, 1, nqp, 1)
  auto re = (re_qp * JxWxT).sum(2);                          // (nelem, ndofe, 3)

  // assemble residual, one displacement component at a time
  for (auto i : make_range(_ndisp))
  {
    auto re_i = re.select(-1, i).contiguous().cpu(); // (nelem, ndofe)
    const auto & dofmap_i = _fe.getGlobalDofMap(_disp_vars[i]);
    _residual->add_vector(re_i.data_ptr<Real>(), dofmap_i);
  }
  _residual->close();
}

#endif // NEML2_ENABLED
