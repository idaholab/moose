//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MFEM_ENABLED

#pragma once
#include "libmesh/ignore_warnings.h"
#include "mfem.hpp"
#include "libmesh/restore_warnings.h"
#include "MFEMGeneralUserObject.h"

/**
 * Constructs and stores an mfem::ParFiniteElementSpace object. Access using the
 * getFESpace() accessor.
 */
class MFEMFESpace : public MFEMGeneralUserObject
{
public:
  static InputParameters validParams();

  MFEMFESpace(const InputParameters & parameters);

  // Note: The simplest way to handle the boilerplate of constructing
  // FiniteElementCollection and FiniteElementSpace objects in the
  // base class while deferring their arguments to the subclasses was
  // to build them after construction was finished. Rather than
  // requiring the user to call an additional Init() function (which
  // could easily be forgotten) instead they get built lazily, when
  // required.

  /// Returns a shared pointer to the constructed fespace.
  inline std::shared_ptr<mfem::ParFiniteElementSpace> getFESpace() const
  {
    if (!_fespace)
      buildFESpace(getVDim());
    return _fespace;
  }

  /// Returns a shared pointer to the constructed fec.
  inline std::shared_ptr<mfem::FiniteElementCollection> getFEC() const
  {
    if (!_fec)
      buildFEC(getFECName());
    return _fec;
  }

  virtual bool isScalar() const = 0;

  virtual bool isVector() const = 0;

protected:
  /// Type of ordering of the vector dofs when _vdim > 1.
  const int _ordering;

  /// Get the name of the desired FECollection.
  virtual std::string getFECName() const = 0;

  /// Get the number of degrees of freedom per basis function needed
  /// in this finite element space.
  virtual int getVDim() const = 0;

  /// Get the quadrature basis enum associated with the given name.
  int getBasis(const std::string & basis_name) const;

private:
  /// Constructs the fec from the fec name.
  void buildFEC(const std::string & fec_name) const;

  /// Stores the constructed fecollection
  mutable std::shared_ptr<mfem::FiniteElementCollection> _fec{nullptr};

  /// Constructs the fespace.
  void buildFESpace(const int vdim) const;
  /// Stores the constructed fespace.
  mutable std::shared_ptr<mfem::ParFiniteElementSpace> _fespace{nullptr};

  /// Mesh FESpace is defined with respect to. May differ from main problem mesh if
  /// FESpace is defined on an MFEMSubMesh.
  mfem::ParMesh & _pmesh;
};

#endif
