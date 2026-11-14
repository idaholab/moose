//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FunctorMaterial.h"

class MooseMesh;
class ElemInfo;

/**
 * Publishes the continuum surface force of a diffuse interface: the interface curvature, and the
 * surface tension force density sigma * kappa * grad(alpha) that a momentum equation consumes
 * (Brackbill, Kothe and Zemach 1992).
 *
 * The curvature is kappa = -div(n) with n = grad(alpha) / |grad(alpha)|, so that a drop of
 * alpha = 1 has positive curvature and the force points into it, toward the higher pressure.
 * The divergence is taken over the faces of a cell, which is why this is a least-squares
 * reconstruction rather than a functor gradient: the normal has to be assembled per face before
 * being differenced. The Rhie-Chow object reads the force density on faces, projects it on the
 * face normal and carries it into the pressure equation as a flux, next to the hydrostatic
 * force, so that at equilibrium the pressure jump balances it face by face.
 */
class ConservativeDiffuseInterfaceSurfaceTensionFunctorMaterial : public FunctorMaterial
{
public:
  static InputParameters validParams();

  ConservativeDiffuseInterfaceSurfaceTensionFunctorMaterial(const InputParameters & parameters);

protected:
  /// The interface curvature of one cell, as the divergence of the unit normal over its faces.
  Real cellCurvature(const ElemInfo & elem_info, const Moose::StateArg & time_arg) const;
  /// The same, given the cell's volume-fraction gradient
  Real cellCurvature(const ElemInfo & elem_info,
                     const RealVectorValue & elem_gradient,
                     const Moose::StateArg & time_arg) const;

  /// The surface tension force density of one cell.
  RealVectorValue cellForceDensity(const ElemInfo & elem_info,
                                   const Moose::StateArg & time_arg) const;

  /// Curvature at an argument that names a single cell.
  template <typename SpaceArg>
  Real curvature(const SpaceArg & r, const Moose::StateArg & t) const
  {
    return cellCurvature(_moose_mesh.elemInfo(r.elem->id()), t);
  }

  /// Curvature on a face, interpolated from the cells it couples.
  Real curvature(const Moose::FaceArg & r, const Moose::StateArg & t) const;

  /// Both quantities are cell based, so there is no nodal evaluation of them.
  Real curvature(const Moose::NodeArg &, const Moose::StateArg &) const;
  /// Nor an evaluation at a bare point, which names no cell
  Real curvature(const Moose::PointArg &, const Moose::StateArg &) const;

  /// Force density at an argument that names a single cell.
  template <typename SpaceArg>
  RealVectorValue forceDensity(const SpaceArg & r, const Moose::StateArg & t) const
  {
    return cellForceDensity(_moose_mesh.elemInfo(r.elem->id()), t);
  }

  /// Force density on a face, interpolated from the cells it couples.
  RealVectorValue forceDensity(const Moose::FaceArg & r, const Moose::StateArg & t) const;

  /// Errors: the force density is cell based and has no nodal value
  RealVectorValue forceDensity(const Moose::NodeArg &, const Moose::StateArg &) const;
  /// Errors: the force density is cell based and has no value at a bare point
  RealVectorValue forceDensity(const Moose::PointArg &, const Moose::StateArg &) const;

  /// A cell quantity on a face: the boundary cell's value on a boundary face, the average of the
  /// two cells' values on an internal one
  template <typename T, typename CellValue>
  T faceValue(const Moose::FaceArg & r, const CellValue & cell_value) const;

  /// Whether a face couples two cells this material is defined on.
  bool isInternalFace(const FaceInfo & fi) const;
  /// The cell on the side of a face where the volume fraction is defined
  const ElemInfo & boundaryElemInfo(const FaceInfo & fi) const;

  /// The volume fraction whose interface this describes
  const Moose::Functor<Real> & _volume_fraction;

  /// The surface tension coefficient
  const Moose::Functor<Real> & _surface_tension_coefficient;

  /// Added to |grad(alpha)| when forming the unit normal, so a vanishing gradient is finite
  const Real _normal_regularization;

  const MooseMesh & _moose_mesh;
  const unsigned int _dim;
};
