//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralUserObject.h"
#include "TaggingInterface.h"
#include "DynamicLibraryLoader.h"
#include "AbaqusUELFunction.h"
#include "libmesh/dense_matrix.h"
#include "libmesh/dense_vector.h"

#include <array>
#include <map>
#include <vector>

/**
 * Common base for user objects that call an Abaqus UEL plugin. It loads the plugin and owns the
 * per-element state variables, stored in a \p Map keyed by \p Key, and the per-element energies,
 * stored in a std::map keyed by \p Key.
 */
template <typename Key, template <typename...> class Map>
class AbaqusUELUserObjectBase : public GeneralUserObject, public TaggingInterface
{
public:
  /// function type for the external UEL plugin function
  using uel_t = Abaqus::UELFunction;

  static InputParameters validParams();
  AbaqusUELUserObjectBase(const InputParameters & params, int nstatev);

  virtual void timestepSetup() override;

  /// energy of the given element, or nullptr if this object does not hold that element
  const std::array<Real, 8> * getUELEnergy(Key element_id) const;

  /// LFLAGS(3) for the current evaluation: 0 for residual and Jacobian, 4 for the Jacobian only,
  /// 5 for the residual only, and 0 for neither
  static int computeLFlag3(bool do_residual, bool do_jacobian);

  /// update the minimum recommended time increment ratio, ignoring a ratio of 0
  static void updatePNewDt(Real & current_min, Real pnewdt);

  /**
   * Add the element residual and Jacobian returned by the UEL plugin. The plugin residual \p re
   * is the negative of the MOOSE residual, and \p ke is column-major, so it is transposed into
   * \p ke_T before it is added.
   */
  void assembleUEL(Assembly & assembly,
                   const DenseVector<Real> & re,
                   const DenseMatrix<Real> & ke,
                   DenseMatrix<Real> & ke_T,
                   const std::vector<dof_id_type> & dofs,
                   bool do_residual,
                   bool do_jacobian);

private:
  /// The plugin file name
  const FileName _plugin;

  /// The plugin library wrapper
  DynamicLibraryLoader _library;

protected:
  /// Function pointer to the dynamically loaded function
  const uel_t _uel;

  /// stateful data
  const int _nstatev;
  std::array<Map<Key, std::vector<Real>>, 2> & _statev;
  std::size_t & _statev_index_current;
  std::size_t & _statev_index_old;

private:
  /// time step at which the state data was last swapped
  int & _t_step_old;

protected:
  /// energy data
  const bool _use_energy;
  std::map<Key, std::array<Real, 8>> & _energy;
  std::map<Key, std::array<Real, 8>> & _energy_old;

  /// recommended time increment ratio (minimum over all elements)
  Real _pnewdt;
};
