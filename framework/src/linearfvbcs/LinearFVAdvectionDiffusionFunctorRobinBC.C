//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVAdvectionDiffusionFunctorRobinBC.h"

registerMooseObject("MooseApp", LinearFVAdvectionDiffusionFunctorRobinBC);

InputParameters
LinearFVAdvectionDiffusionFunctorRobinBC::validParams()
{
  InputParameters params = LinearFVAdvectionDiffusionBC::validParams();
  params.addClassDescription(
      "Adds a Robin BC of the form alpha* grad_phi*n + beta*phi = gamma, "
      "which can be used for the assembly of linear "
      "finite volume system and whose face values are determined using three functors. This kernel is "
      "only designed to work with advection-diffusion problems.");
  params.addRequiredParam<MooseFunctorName>("alpha", "The functor which is the coefficient of the normal gradient term.");
  params.addRequiredParam<MooseFunctorName>("beta", "The functor which is the coefficient of the scalar term.");
  params.addRequiredParam<MooseFunctorName>("gamma", "The functor which is the constant term on the RHS of the Robin BC expression.");
  return params;
}

LinearFVAdvectionDiffusionFunctorRobinBC::LinearFVAdvectionDiffusionFunctorRobinBC(
    const InputParameters & parameters)
  : LinearFVAdvectionDiffusionBC(parameters), 
    _alpha(getFunctor<Real>("alpha")),
    _beta(getFunctor<Real>("beta" )),
    _gamma(getFunctor<Real>("gamma"))
{
}

RealVectorValue
LinearFVAdvectionDiffusionFunctorRobinBC::computeOrthogonalProjectionVector() const
{
  const auto nhat = _current_face_info->normal();

//  // option 1: PG/AC
//  const auto d_cb = computeCellToFaceVector();
//  
//  return d_cb*nhat*nhat; // projection of d_cb along surface normal, denominator |d_cb| cancels out

  // option 2: Moukalled
   RealVectorValue d_cb_hat = computeCellToFaceVector() /
          std::sqrt(computeCellToFaceVector()*computeCellToFaceVector());
  
   return _current_face_info->faceArea()/(nhat*d_cb_hat) * d_cb_hat; // corresponds to over-relaxed 
}

RealVectorValue
LinearFVAdvectionDiffusionFunctorRobinBC::computeNonOrthogonalProjectionVector() const
{
  return ( _current_face_info->normal() * _current_face_info->faceArea() ) -
     computeOrthogonalProjectionVector();
}

Real
LinearFVAdvectionDiffusionFunctorRobinBC::computePhiDenomFactor() const
{
  const auto face = singleSidedFaceArg(_current_face_info);
  const auto state = determineState();

  const auto alpha = _alpha(face, state);
  const auto beta  = _beta(face, state);

  const Real d_o_mag   = std::sqrt(computeOrthogonalProjectionVector()
             * computeOrthogonalProjectionVector() );
  const Real d_cb_mag  = std::sqrt(computeCellToFaceVector()
             * computeCellToFaceVector() );

  const auto d_no = computeNonOrthogonalProjectionVector();

  const auto nhat = _current_face_info->normal();
  const auto d_cb_hat = computeCellToFaceVector()/d_cb_mag;
  const auto d_t_hat = nhat - ( (d_cb_hat * nhat) * nhat);
  const Real d_s_cos = d_cb_hat * nhat;

  // return (alpha * d_o_mag) + (beta * d_cb_mag);
  return (alpha ) + (beta * d_cb_mag * d_s_cos);
}

Real
LinearFVAdvectionDiffusionFunctorRobinBC::computeBoundaryValue() const
{

  const auto face = singleSidedFaceArg(_current_face_info);
  const auto elem_arg = makeElemArg(_current_face_type == FaceInfo::VarFaceNeighbors::ELEM
                                        ? _current_face_info->elemPtr()
                                        : _current_face_info->neighborPtr());
  const auto state = determineState();

  const auto alpha = _alpha(face, state);
  const auto gamma = _gamma(face, state);

  const auto phi = raw_value(_var(elem_arg, state));
  const auto grad_phi = _var.gradSln(*_current_face_info->elemInfo());

  const Real d_o_mag   = std::sqrt(computeOrthogonalProjectionVector()
             * computeOrthogonalProjectionVector() );
  const Real d_cb_mag  = std::sqrt(computeCellToFaceVector()
             * computeCellToFaceVector() );
  const auto d_no = computeNonOrthogonalProjectionVector();

  const auto nhat = _current_face_info->normal();
  const auto d_cb_hat = computeCellToFaceVector()/d_cb_mag;
  const auto d_t_hat = nhat - ( (d_cb_hat * nhat) * nhat);
  const Real d_s_cos = d_cb_hat * nhat;

  return ( (alpha * d_o_mag * phi) 
           - ( alpha * d_cb_mag * grad_phi * d_no)
           + (gamma * d_cb_mag)
         ) / computePhiDenomFactor();

//  return ( (alpha *  phi) 
//           + ( alpha * d_cb_mag * grad_phi * d_t_hat)
//           + (gamma * d_cb_mag * d_s_cos)
//         ) / computePhiDenomFactor();
}

Real
LinearFVAdvectionDiffusionFunctorRobinBC::computeBoundaryNormalGradient() const
{

  const auto face = singleSidedFaceArg(_current_face_info);
  const auto state = determineState();

  const auto elem_arg = makeElemArg(_current_face_type == FaceInfo::VarFaceNeighbors::ELEM
                                        ? _current_face_info->elemPtr()
                                        : _current_face_info->neighborPtr());

  const auto alpha = _alpha(face, state);
  const auto beta  = _beta(face, state);
  const auto gamma = _gamma(face, state);
  const auto phi   = raw_value(_var(elem_arg, state));

  return (gamma - beta * phi) / alpha;
}

// implicit terms for advection kernel
Real
LinearFVAdvectionDiffusionFunctorRobinBC::computeBoundaryValueMatrixContribution() const
{
  const auto face = singleSidedFaceArg(_current_face_info);
  const auto state = determineState();

  const auto alpha = _alpha(face, state);
  const Real d_o_mag   = std::sqrt(computeOrthogonalProjectionVector()
             * computeOrthogonalProjectionVector() );

  // std::cout<<"Adv. Imp. Term 1: "<< alpha * d_o_mag / (computePhiDenomFactor()) <<std::endl;
  // return alpha * d_o_mag / computePhiDenomFactor();

  std::cout<<"Adv. Imp. Term 1: "<< alpha / (computePhiDenomFactor()) <<std::endl;
  return alpha / computePhiDenomFactor();
}

// explicit terms for advection kernel
Real
LinearFVAdvectionDiffusionFunctorRobinBC::computeBoundaryValueRHSContribution() const
{
  const auto face = singleSidedFaceArg(_current_face_info);
  const auto state = determineState();

  const auto alpha = _alpha(face, state);
  const auto gamma = _gamma(face, state);
  const auto grad_phi = _var.gradSln(*_current_face_info->elemInfo());

  const auto d_no = computeNonOrthogonalProjectionVector();
  const auto d_no_hat = std::sqrt(d_no * d_no);
  const Real d_cb_mag  = std::sqrt(computeCellToFaceVector()
             * computeCellToFaceVector() );

  // option 1: PG/AC
  const auto nhat = _current_face_info->normal();
  const auto d_cb_hat = computeCellToFaceVector()/d_cb_mag;
  const auto d_t_hat = nhat - ( (d_cb_hat * nhat) * nhat);
  const Real d_s_cos = d_cb_hat * nhat;

  std::cout<<"Adv. Exp. Term 1: "<<( alpha * d_cb_mag * grad_phi * d_t_hat)<<std::endl;
  std::cout<<"Adv. Exp. Term 2: "<<(gamma * d_cb_mag * d_s_cos)<<std::endl;
  std::cout<<"Adv. Exp. Term 3: "<<computePhiDenomFactor()<<std::endl;
  return ( ( alpha * d_cb_mag * grad_phi * d_t_hat) + (gamma * d_cb_mag * d_s_cos)
         ) / computePhiDenomFactor();

//  const Real term1 = alpha * d_cb_mag * grad_phi * d_no;
//  const Real term2 = gamma * d_cb_mag;
//  const Real term3 = computePhiDenomFactor();
//
//  // option 2: Moukalled
//  std::cout<<"Adv. Exp. Term 1: "<<( - alpha * d_cb_mag * grad_phi * d_no)<<std::endl;
//  std::cout<<"Adv. Exp. Term 2: "<<(gamma * d_cb_mag)<<std::endl;
//  std::cout<<"Adv. Exp. Term 3: "<<computePhiDenomFactor()<<std::endl;
//  return ( ( - alpha * d_cb_mag * grad_phi * d_no) + (gamma * d_cb_mag)
//         ) / computePhiDenomFactor();
}

// implicit terms for diffusion kernel
Real
LinearFVAdvectionDiffusionFunctorRobinBC::computeBoundaryGradientMatrixContribution() const
{
  const auto face = singleSidedFaceArg(_current_face_info);
  const auto state = determineState();

  const auto alpha  = _alpha(face, state);
  const auto beta   =  _beta(face, state);

  const Real d_o_mag   = std::sqrt(computeOrthogonalProjectionVector()
             * computeOrthogonalProjectionVector() );

  std::cout<<"Diff. Imp. Term 1: "<< -beta / (computePhiDenomFactor()) <<std::endl;
  return   beta / alpha; //computePhiDenomFactor();

  // option 2: WIP
//  std::cout<<"Diff. Imp. Term 1: "<< -beta * d_o_mag/ computePhiDenomFactor() <<std::endl;
//  return -beta * d_o_mag/computePhiDenomFactor();
}

// explicit terms for diffusion kernel
Real
LinearFVAdvectionDiffusionFunctorRobinBC::computeBoundaryGradientRHSContribution() const
{
  // The boundary term from the ce ntral difference approximation of the
  // normal gradient.
  const auto face = singleSidedFaceArg(_current_face_info);
  const auto state = determineState();
  const auto grad_phi = _var.gradSln(*_current_face_info->elemInfo());

  const auto alpha = _alpha(face, state);
  const auto beta  =  _beta(face, state);
  const auto gamma = _gamma(face, state);

  const Real d_cb_mag  = std::sqrt(computeCellToFaceVector()
             * computeCellToFaceVector() );
  const auto d_no = computeNonOrthogonalProjectionVector();
  const Real dnom = computePhiDenomFactor();


  // option 1: PG/AC
  const auto nhat = _current_face_info->normal();
  const auto d_cb_hat = computeCellToFaceVector()/d_cb_mag;
  const auto d_t_hat = nhat - ( (d_cb_hat * nhat) * nhat);
  const Real d_s_cos = d_cb_hat * nhat;
 
  std::cout<<"Adv. Exp. Term 1: "<<( - (beta * d_cb_mag * grad_phi * d_t_hat) / dnom)<<std::endl;
  std::cout<<"Adv. Exp. Term 2: "<<- ( beta * gamma * d_cb_mag * d_s_cos /alpha / dnom )<<std::endl;
  std::cout<<"Adv. Exp. Term 3: "<<(gamma / alpha)<<std::endl;
 
  return  0.0*( - (beta * d_cb_mag * grad_phi * d_t_hat) / dnom)
        - 0.0*( beta * gamma * d_cb_mag * d_s_cos /alpha / dnom )
        - (gamma / alpha);

  // option 2: WIP
//  std::cout<<"Adv. Exp. Term 1: "<< ( (beta * d_cb_mag * grad_phi * d_no) / dnom)<<std::endl;
//  std::cout<<"Adv. Exp. Term 2: "<< - ( beta * gamma * d_cb_mag /alpha / dnom )<<std::endl;
//  std::cout<<"Adv. Exp. Term 3: "<<(gamma / alpha)<<std::endl;
//  return  ( (beta * d_cb_mag * grad_phi * d_no) / dnom)
//        - ( beta * gamma * d_cb_mag /alpha / dnom )
//        + (gamma / alpha);
}
