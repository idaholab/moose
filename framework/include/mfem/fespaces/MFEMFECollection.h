#pragma once
#include "MFEMGeneralUserObject.h"
#include "mfem.hpp"
#include <string>

/**
 * Constructs and stores an mfem::FiniteElementCollection object. Access using
 * the getFEC() accessor.
 */
class MFEMFECollection : public MFEMGeneralUserObject
{
public:
  static InputParameters validParams();

  MFEMFECollection(const InputParameters & parameters);

  ~MFEMFECollection() override = default;

  /// Returns a shared pointer to the constructed fec.
  inline std::shared_ptr<mfem::FiniteElementCollection> getFEC() const { return _fec; }

  /// Returns the vdim needed for a finite element space correctly to
  /// represent the desired dimension of vectors for this finite
  /// element collection. Note that this may be different from the
  /// logical vector dimension of the finite element space.
  int getFESpaceVDim() const;

protected:
  const int _fec_order;
  const int _fec_dim;
  const int _fec_vdim;
  const std::string _fec_type;
  const std::string _fec_name;

private:
  /// Constructs the fec name from the order and fespace type. Note
  /// this only works for H1 and L2 spaces or when the vector
  /// dimension of ND or RT spaces is equal to the spatial dimension.
  const std::string buildFECName();

  /// Constructs the fec from the fec name.
  const std::shared_ptr<mfem::FiniteElementCollection> buildFEC();

  /// Stores the constructed fec.
  const std::shared_ptr<mfem::FiniteElementCollection> _fec{nullptr};
};
