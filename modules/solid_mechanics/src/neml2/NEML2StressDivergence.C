//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef NEML2_ENABLED

// NEML2 includes
#include "neml2/tensors/functions/stack.h"
#include "neml2/tensors/functions/sum.h"

// MOOSE includes
#include "NEML2StressDivergence.h"

registerMooseObject("SolidMechanicsApp", NEML2StressDivergence);

InputParameters
NEML2StressDivergence::validParams()
{
  InputParameters params = NEML2PostKernel::validParams();
  params.addClassDescription(
      "This user object calculates the stress divergence for a given set of displacement "
      "variables and assemble the contribution into the residual vector.");
  params.addRequiredParam<std::vector<NonlinearVariableName>>(
      "displacements",
      "The displacements variables whose function space will be used to define the test "
      "functions.");
  params.addRequiredParam<std::string>("stress",
                                       "The name of the NEML2 variable to use as the stress.");
  params.addRequiredParam<std::string>("residual", "The tag for the residual");
  return params;
}

NEML2StressDivergence::NEML2StressDivergence(const InputParameters & parameters)
  : NEML2PostKernel(parameters),
    _residual(dynamic_cast<PetscVector<Real> *>(&_sys.system().add_vector(
        getParam<std::string>("residual"), false, libMesh::ParallelType::GHOSTED))),
    _stress(
        _constitutive.getOutput(NEML2Utils::parseVariableName(getParam<std::string>("stress")))),
    _disp_vars(getParam<std::vector<NonlinearVariableName>>("displacements")),
    _ndisp(_disp_vars.size())
{
  mooseAssert(_residual, "Failed to cast residual to PetscVector<Real>");

  if (_disp_vars.size() < 1 || _disp_vars.size() > 3)
    mooseError("NEML2StressDivergence requires 1 to 3 displacement variables, got ",
               _disp_vars.size());

  _grad_test_x = &_fe.getPhiGradient(_disp_vars[0]);
  _grad_test_y = _ndisp >= 2 ? &_fe.getPhiGradient(_disp_vars[1]) : nullptr;
  _grad_test_z = _ndisp >= 3 ? &_fe.getPhiGradient(_disp_vars[2]) : nullptr;
}

void
NEML2StressDivergence::forward()
{
  if (!_constitutive.outputReady())
    return;

  torch::InferenceMode guard;

  auto dphix = neml2::Tensor(*_grad_test_x, 3);
  auto dphiy = _ndisp >= 2 ? neml2::Tensor(*_grad_test_y, 3) : neml2::Tensor::zeros_like(dphix);
  auto dphiz = _ndisp >= 3 ? neml2::Tensor(*_grad_test_z, 3) : neml2::Tensor::zeros_like(dphix);
  auto dphi = neml2::base_stack({dphix, dphiy, dphiz}, 0);          // (nelem, ndofe, nqp; 3, 3)
  auto stress = neml2::R2(neml2::SR2(_stress)).batch_unsqueeze(-2); // (nelem, 1,     nqp; 3, 3)

  // weak form
  auto re_qp = neml2::base_sum(dphi * neml2::Tensor(stress), -1); // (nelem, ndofe, nqp; 3)

  // element integration
  auto JxWxT = _neml2_assembly.JxWxT().batch_unsqueeze(-2).base_unsqueeze(0); // (nelem, 1, nqp; 1)
  auto re = neml2::batch_sum(re_qp * JxWxT, -1)
                .base_index({neml2::indexing::Slice(0, _ndisp)}); // (nelem, ndofe; ndisp)

  // assemble residual
  for (auto i : make_range(_ndisp))
  {
    auto re_i = re.base_index({i}).contiguous().cpu();
    const auto & dofmap_i = _fe.getGlobalDofMap(_disp_vars[i]);
    _residual->add_vector(re_i.data_ptr<Real>(), dofmap_i);
  }
  _residual->close();
}

#endif
