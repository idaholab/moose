#pragma once
#include "MFEMGeneralUserObject.h"
#include "MFEMFECollection.h"

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
  inline std::shared_ptr<mfem::FiniteElementCollection> getFEC() const { return _fec.getFEC(); }

protected:
  /// Vector dimension (number of unknowns per degree of freedom).
  const int _vdim;

  /// Type of ordering of the vector dofs when _vdim > 1.
  const int _ordering;

  /// Constructs and stores the fec.
  const MFEMFECollection _fec;

private:
  /// Constructs the fespace.
  const std::shared_ptr<mfem::ParFiniteElementSpace> buildFESpace();

  /// Stores the constructed fespace.
  const std::shared_ptr<mfem::ParFiniteElementSpace> _fespace{nullptr};
};
