#include "MFEMLinearElasticityKernel.h"

registerMooseObject("MooseApp", MFEMLinearElasticityKernel);

InputParameters
MFEMLinearElasticityKernel::validParams()
{
  InputParameters params = MFEMKernel::validParams();
  params.addClassDescription(
      "The isotropic linear elasticity operator with weak form "
      "$(c_{ikjl} \\nabla u_j, \\nabla v_i)$, to be added to an MFEM problem, where "
      "$c_{ikjl}$ is the isotropic elasticity tensor, "
      "$c_{ikjl} = \\lambda \\delta_{ik} \\delta_{jl} + \\mu \\left( \\delta_{ij} \\delta_{kl} + "
      "\\delta_{il} \\delta_{jk} \\right)$, "
      "$\\lambda$ is the first Lamé parameter, $\\lambda = \\frac{E\\nu}{(1-2\\nu)(1+\\nu)}$, "
      "$\\mu$ is the second Lamé parameter, $\\mu = \\frac{E}{2(1+\\nu)}$, "
      "where $E$ is Young's modulus and $\\nu$ is Poisson's ratio.");

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
