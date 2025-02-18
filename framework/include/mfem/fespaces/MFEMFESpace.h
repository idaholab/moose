#pragma once
#include "mfem.hpp"
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

  /// Returns a shared pointer to the constructed fespace.
  inline std::shared_ptr<mfem::ParFiniteElementSpace> getFESpace() const { return _fespace; }

  /// Returns a shared pointer to the constructed fec.
  inline std::shared_ptr<mfem::FiniteElementCollection> getFEC() const { return _fec; }

protected:
  /// Order of the basis functions in the finite element collection
  const int _fec_order;

  /// Name of the family of finite element collections to use
  const std::string _fec_type;

  /// Type of ordering of the vector dofs when _vdim > 1.
  const int _ordering;

  /// Number of vector components (may differ from vector dimension)
  const int _vec_components;

  /// Vector dimension (number of unknowns per degree of freedom).
  const int _vdim;

  /// Dimension of the problem (i.e., the highest dimension of the reference elements in the mesh)
  const int _problem_dim;

private:
  /// Constructs the fec name from the order and fespace type. Note
  /// this only works for H1 and L2 spaces or when the vector
  /// dimension of ND or RT spaces is equal to the spatial dimension.
  const std::string buildFECName() const;

  /// Constructs the fec from the fec name.
  const std::shared_ptr<mfem::FiniteElementCollection> buildFEC() const;

  /// Stores the constructed fecollection
  const std::shared_ptr<mfem::FiniteElementCollection> _fec;

  /// Constructs the fespace.
  const std::shared_ptr<mfem::ParFiniteElementSpace> buildFESpace();

  /// Stores the constructed fespace.
  const std::shared_ptr<mfem::ParFiniteElementSpace> _fespace{nullptr};
};
