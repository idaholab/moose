#include "DarcyVelocity.h"

#include "metaphysicl/raw_type.h"

registerMooseObject("BabblerApp", DarcyVelocity);

InputParameters
DarcyVelocity::validParams()
{
  InputParameters params = VectorAuxKernel::validParams();
  params.addClassDescription("Compute volumetric flux (which has units of velocity) for a given "
                             "pressure gradient, permeability, and viscosity using Darcy's Law: "
                             "$\\vec{u} = -\\frac{\\mathbf{K}}{\\mu} \\nabla p$");

  // Required parameter for specifying the MooseVariable to couple to, i.e., pressure
  params.addRequiredCoupledVar("pressure", "The pressure field.");

  return params;
}

DarcyVelocity::DarcyVelocity(const InputParameters & parameters)
  : VectorAuxKernel(parameters),
    _grad_p(coupledGradient("pressure")),

    // Note that only AuxKernels operating on elemental AuxVariables can consume a MaterialProperty
    // reference, since they are defined at the Gauss QPs within the element
    _permeability(getADMaterialProperty<Real>("permeability")),
    _viscosity(getADMaterialProperty<Real>("viscosity"))
{
}

RealVectorValue
DarcyVelocity::computeValue()
{
  // Access the gradient of the pressure at the QP. The MetaPhysicL::raw_value() method will return
  // the computed value from an automatically differentiable type, like ADMaterialProperty, as
  // opposed to any of its gradients WRT the spatial domain, which is what we want in this case.
  return -MetaPhysicL::raw_value(_permeability[_qp] / _viscosity[_qp]) * _grad_p[_qp];
}
