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

protected:
  const int _fec_order;
  const std::string _fec_type;
  const std::string _fec_name;

private:
  /// Constructs the fec name from the order and fespace type.
  const std::string buildFECName();

  /// Constructs the fec from the fec name.
  const std::shared_ptr<mfem::FiniteElementCollection> buildFEC();

  /// Stores the constructed fec.
  const std::shared_ptr<mfem::FiniteElementCollection> _fec{nullptr};
};