#include "MFEMLinearElasticityKernel.h"

registerMooseObject("PlatypusApp", MFEMLinearElasticityKernel);

InputParameters
MFEMLinearElasticityKernel::validParams()
{
  InputParameters params = MFEMKernel::validParams();
  params.addClassDescription(
      "The Cauchy stress tensor $\\sigma(u)_{ij} = \\lambda \\partial_k u_k \\delta_{ij}"
      "+ \\mu (\\partial_j u_i + \\partial_i u_j)$ with the weak form of"
      "$(\\sigma(\\phi_i), \\sigma(u_h))$, to be added to an MFEM problem");

  params.addParam<std::string>(
      "lambda", "Name of MFEM Lame constant lambda to multiply the div(u)*I term by");
  params.addParam<std::string>("mu",
                               "Name of MFEM Lame constant mu to multiply the gradients term by");

  return params;
}

MFEMLinearElasticityKernel::MFEMLinearElasticityKernel(const InputParameters & parameters)
  : MFEMKernel(parameters),
    _lambda_name(getParam<std::string>("lambda")),
    _mu_name(getParam<std::string>("mu")),
    _lambda(getMFEMProblem().getProperties().getScalarProperty(_lambda_name)),
    _mu(getMFEMProblem().getProperties().getScalarProperty(_mu_name))
{
}

mfem::BilinearFormIntegrator *
MFEMLinearElasticityKernel::createIntegrator()
{
  return new mfem::ElasticityIntegrator(_lambda, _mu);
}
