#ifdef MFEM_ENABLED

#pragma once
#include "MFEMKernel.h"

// clang-format off
/*
 * \f[
 * (c_{ikjl} \nabla u_j, \nabla v_i),
 * c_{ikjl} = \lamba \delta_{ik} \delta_{jl} + \mu (\delta_{ij} \delta_{kl} + \delta_{il} \delta_{jk}),
 * \lambda = (E\nu)/((1-2\nu)(1+\nu)),
 * \mu = E/(2(1+\nu)),
 * E is Young's modulus,
 * \nu is Poisson's ratio
 * \f]
*/
// clang-format on
class MFEMLinearElasticityKernel : public MFEMKernel<mfem::BilinearFormIntegrator>
{
public:
  static InputParameters validParams();

  MFEMLinearElasticityKernel(const InputParameters & parameters);

  virtual mfem::BilinearFormIntegrator * createIntegrator() override;

protected:
  std::string _lambda_name;
  std::string _mu_name;
  mfem::Coefficient & _lambda;
  mfem::Coefficient & _mu;
};

#endif
