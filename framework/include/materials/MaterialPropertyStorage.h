//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Moose.h"
#include "MooseHashing.h"
#include "HashMap.h"
#include "MaterialProperty.h"
#include "MaterialPropertyRegistry.h"
#include "MaterialData.h"

#include <variant>

// Forward declarations
class MaterialBase;
class QpMap;

// libMesh forward declarations
namespace libMesh
{
class QBase;
class Elem;
}

class MaterialPropertyStorage;

void dataStore(std::ostream & stream, MaterialPropertyStorage & storage, void * context);
void dataLoad(std::istream & stream, MaterialPropertyStorage & storage, void * context);

/**
 * Stores the stateful material properties computed by materials.
 *
 * Thread-safe
 */
class MaterialPropertyStorage
{
public:
  MaterialPropertyStorage(MaterialPropertyRegistry & registry);

  /**
   * Basic structure for storing information about a property
   */
  struct PropRecord
  {
    /// Whether or not this property is stateful
    bool stateful() const { return state > 0; }
    /// The material (type,name) that have declared this property
    std::set<std::string> declarers;
    /// The type of this property
    std::string type;
    /// The stateful id in _storage used for this property, if any
    unsigned int stateful_id = invalid_uint;
    /// The max state requrested for this property (0 = current, 1 = old, ...)
    unsigned int state = 0;
    /// Whether or not this property was restored (stateful only)
    bool restored = false;
  };

  /**
   * Creates storage for newly created elements from mesh Adaptivity.  Also, copies values from the
   * parent qps to the new children.
   *
   * Note - call this on the MaterialPropertyStorage object for the _children_ that you want to
   * project to.  ie, if you are trying
   * to project to the sides of the children, then call this on the boundary
   * MaterialPropertyStorage.  Pass in the parent
   * MaterialPropertyStorage you are projecting _from_.  ie the volume one if you are projecting to
   * "internal" child element faces.
   *
   * There are 3 cases here:
   *
   * 1. Volume to volume (parent_side = -1, child = -1, child_side = -1)
   *    Call on volume MaterialPropertyStorage and pass volume MaterialPropertyStorage for
   * parent_material_props
   *
   * 2. Parent side to child side (parent_side = 0+, child = -1, child_side = 0+) where parent_side
   * == child_side
   *    Call on boundary MaterialPropertyStorage and pass boundary MaterialPropertyStorage for
   * parent_material_props
   *
   * 3. Child side to parent volume (parent_side = -1, child = 0+, child_side = 0+)
   *    Call on boundary MaterialPropertyStorage and pass volume MaterialPropertyStorage for
   * parent_material_props
   *
   * @param pid - processor id of children to prolong to
   * @param refinement_map - 2D array of QpMap objects
   * @param qrule The current quadrature rule
   * @param qrule_face The current face qrule
   * @param parent_material_props The place to pull parent material property values from
   * @param tid The thread ID
   * @param elem The parent element that was just refined
   * @param input_parent_side - the side of the parent for which material properties are prolonged
   * @param input_child - the number of the child
   * @param input_child_side - the side on the child where material properties will be prolonged
   */
  void prolongStatefulProps(processor_id_type pid,
                            const std::vector<std::vector<QpMap>> & refinement_map,
                            const libMesh::QBase & qrule,
                            const libMesh::QBase & qrule_face,
                            MaterialPropertyStorage & parent_material_props,
                            const THREAD_ID tid,
                            const Elem & elem,
                            const int input_parent_side,
                            const int input_child,
                            const int input_child_side);

  /**
   * Based on the p-refinement flag of \p elem, either prolong (for refinement) or restrict (for
   * coarsening) the stateful material property data.
   * @param pid The rank returned by the mesh. This is used to assert that we are only
   * prolonging/restricting for elements that our process owns
   * @param p_refinement_map A map that describes, for each quadrature point on the current level
   * which quadrature point on the previous level it is closest to
   * @param qrule The volumetric quadrature rule
   * @param qrule_face The face quadrature rule
   * @param tid The thread ID
   * @param elem The element that was just p-refined or coarsened that we are performing material
   * property prolongation/restriction for
   * @param input_side Which element side we are performing material property
   * prolongation/restriction for. Pass in -1 for doing volumetric prolongation/restriction
   */
  void updateStatefulPropsForPRefinement(const processor_id_type pid,
                                         const std::vector<QpMap> & p_refinement_map,
                                         const libMesh::QBase & qrule,
                                         const libMesh::QBase & qrule_face,
                                         const THREAD_ID tid,
                                         const Elem & elem,
                                         const int input_side);

  /**
   * Creates storage for newly created elements from mesh Adaptivity.  Also, copies values from the
   * children to the parent.
   *
   * @param coarsening_map - map from unsigned ints to QpMap's
   * @param coarsened_element_children - a pointer to a vector of coarsened element children
   * @param qrule The current quadrature rule
   * @param qrule_face The current face qrule
   * @param tid The thread ID
   * @param elem The parent element that was just refined
   * @param input_side Side of the element 'elem' (0 for volumetric material properties)
   */
  void restrictStatefulProps(const std::vector<std::pair<unsigned int, QpMap>> & coarsening_map,
                             const std::vector<const Elem *> & coarsened_element_children,
                             const libMesh::QBase & qrule,
                             const libMesh::QBase & qrule_face,
                             const THREAD_ID tid,
                             const Elem & elem,
                             int input_side = -1);

  /**
   * Initialize stateful material properties
   * @param tid The thread ID
   * @param mats Materials that will compute the initial values
   * @param n_qpoints Number of quadrature points
   * @param elem Element we are on
   * @param side Side of the element 'elem' (0 for volumetric material properties)
   *
   * If restartable stateful information is available, this will load from restart
   * instead of calling initStatefulProperties()
   */
  void initStatefulProps(const THREAD_ID tid,
                         const std::vector<std::shared_ptr<MaterialBase>> & mats,
                         const unsigned int n_qpoints,
                         const Elem & elem,
                         const unsigned int side = 0);

  /**
   * Shift the material properties in time.
   *
   * Old material properties become older, current material properties become old. Older material
   * properties are
   * reused for computing current properties. This is called when solve succeeded.
   */
  void shift();

  /**
   * Copy material properties from elem_from to elem_to. Thread safe.
   *
   * WARNING: This is not capable of copying material data to/from elements on other processors.
   *          It only works if both elem_to and elem_from are both on the local processor.
   *          We can't currently check to ensure that they're on processor here because this isn't a
   * ParallelObject.
   *
   * @param tid The thread ID
   * @param elem_to Element to copy data to
   * @param elem_from Element to copy data from
   * @param side Side number (elemental material properties have this equal to zero)
   * @param n_qpoints number of quadrature points to work with
   */
  void copy(const THREAD_ID tid,
            const Elem & elem_to,
            const Elem & elem_from,
            unsigned int side,
            unsigned int n_qpoints);

  /**
   * Copy material properties from elem_from to elem_to.
   * Similar to the other method but using pointers to elements instead of references.
   *
   * @param tid The thread ID
   * @param elem_to Pointer to the element to copy data to
   * @param elem_from Pointer to the element to copy data from
   * @param side Side number (elemental material properties have this equal to zero)
   * @param n_qpoints number of quadrature points to work with
   */
  void copy(const THREAD_ID tid,
            const Elem * elem_to,
            const Elem * elem_from,
            unsigned int side,
            unsigned int n_qpoints);

  /**
   * Swap (shallow copy) material properties in MaterialData and MaterialPropertyStorage
   * Thread safe
   * @param tid The thread id
   * @param elem Element id
   * @param side Side number (elemental material properties have this equal to zero)
   */
  void swap(const THREAD_ID tid, const Elem & elem, unsigned int side);

  /**
   * Swap (shallow copy) material properties in MaterialPropertyStorage and MaterialDat
   * Thread safe
   * @param tid The thread id
   * @param elem Element id
   * @param side Side number (elemental material properties have this equal to zero)
   */
  void swapBack(const THREAD_ID tid, const Elem & elem, unsigned int side);

  /**
   * @return a Boolean indicating whether stateful properties exist on this material
   */
  bool hasStatefulProperties() const { return maxState() > 0; }

  /**
   * @return a Boolean indicating whether or not this material has older properties declared
   */
  bool hasOlderProperties() const { return maxState() > 1; }

  /**
   * Accessible type of the stored material property data.
   *
   * This probably should have been returned as a proxy class; only
   * access it via foo[elem][side] and maybe we'll be able to refactor
   * it in the future without breaking your code.
   */
  typedef HashMap<const Elem *, HashMap<unsigned int, MaterialProperties>> PropsType;

  ///@{
  /**
   * Access methods to the stored material property data with the given state \p state.
   */
  const PropsType & props(const unsigned int state = 0) const;
  const MaterialProperties &
  props(const Elem * elem, unsigned int side, const unsigned int state = 0) const;
  MaterialProperties & setProps(const Elem * elem, unsigned int side, const unsigned int state = 0);
  ///@}

  bool hasProperty(const std::string & prop_name) const { return _registry.hasProperty(prop_name); }

  /**
   * Adds a property with the name \p prop_name, type \p type, and state \p state (0 = current, 1 =
   * old, etc)
   *
   * This is idempotent - calling multiple times with the same name will provide the same id and
   * works fine.
   *
   * \p declarer should be specified by the object declaring the property if it is being declared.
   */
  unsigned int addProperty(const std::string & prop_name,
                           const std::type_info & type,
                           const unsigned int state,
                           const MaterialBase * const declarer);

  const std::vector<unsigned int> & statefulProps() const { return _stateful_prop_id_to_prop_id; }

  const MaterialPropertyRegistry & getMaterialPropertyRegistry() const { return _registry; }

  /**
   * @return The name of the stateful property with id \p id, if any.
   */
  std::optional<std::string> queryStatefulPropName(const unsigned int id) const;

  /**
   * Remove the property storage and element pointer from internal data structures
   * Use this when elements are deleted so we don't end up with invalid elem pointers (for e.g.
   * stateful properties) hanging around in our data structures
   */
  void eraseProperty(const Elem * elem);

  /**
   * @returns The current maximum stored state (0 = none, 1 = old, 2 = older)
   */
  unsigned int maxState() const
  {
    mooseAssert(_max_state < _storage.size(), "Too big");
    return _max_state;
  }
  /**
   * @returns The number of stored states (2 = up to old, 3 = up to older)
   */
  unsigned int numStates() const { return maxState() + 1; }
  /**
   * @returns A range over states to be used in range-based for loops
   */
  libMesh::IntRange<unsigned int> stateIndexRange() const
  {
    return libMesh::IntRange<unsigned int>(0, numStates());
  }
  /**
   * @returns A range over stateful states to be used in range-based for loops
   *
   * Will be an empty range if there are no stateful states
   */
  libMesh::IntRange<unsigned int> statefulIndexRange() const
  {
    return libMesh::IntRange<unsigned int>(1, numStates());
  }

  /**
   * @return The MaterialData for thread \p tid
   */
  MaterialData & getMaterialData(const THREAD_ID tid) { return _material_data[tid]; }

  /**
   * Sets the loading of stateful material properties to recover
   *
   * This enforces the requirement of one-to-one stateful material properties,
   * disabling advanced restart of stateful properties
   */
  void setRecovering() { _recovering = true; }

  /**
   * Sets the loading of stateful material properties in place
   *
   * On init, this cannot be set because we must first call initProps()
   * to properly initialize the dynamic types within _storage. After
   * the first sweep through with initProps(), we can then load the stateful
   * props directly in place into _storage
   *
   * Also clears _restartable_map, as it should no longer be needed
   */
  void setRestartInPlace();

  /**
   * Get the property record associated with the material with id \p id
   */
  const PropRecord & getPropRecord(const unsigned int id) const;

  /**
   * @return Whether or not the material property with name \p name was restored
   */
  bool isRestoredProperty(const std::string & name) const;

protected:
  /// The actual storage
  std::array<PropsType, MaterialData::max_state + 1> _storage;

  /// Property records indexed by property id (may be null)
  std::vector<std::optional<PropRecord>> _prop_records;

  /// the vector of stateful property ids (the vector index is the map to stateful prop_id)
  std::vector<unsigned int> _stateful_prop_id_to_prop_id;

  void sizeProps(MaterialProperties & mp, unsigned int size);

private:
  /// Initializes hashmap entries for element and side to proper qpoint and
  /// property count sizes.
  std::vector<MaterialProperties *>
  initProps(const THREAD_ID tid, const Elem * elem, unsigned int side, unsigned int n_qpoints);

  /// Initializes just one hashmap's entries
  MaterialProperties & initProps(const THREAD_ID tid,
                                 const unsigned int state,
                                 const Elem * elem,
                                 unsigned int side,
                                 unsigned int n_qpoints);

  ///@{
  /**
   * Shallow copies of material properties
   *
   */
  static void shallowSwapData(const std::vector<unsigned int> & stateful_prop_ids,
                              MaterialProperties & data,
                              MaterialProperties & data_from);
  static void shallowSwapDataBack(const std::vector<unsigned int> & stateful_prop_ids,
                                  MaterialProperties & data,
                                  MaterialProperties & data_from);
  ///@}

  /**
   * @returns A writeable reference to the properties at state \p state.
   */
  PropsType & setProps(const unsigned int state);

  /**
   * @returns A writeable reference to the properties for elem \p elem,
   * side \p side, and state \p state.
   *
   * Similar to setProps, but will initialize (default construct) the
   * entry if it does not exist.
   */
  MaterialProperties &
  initAndSetProps(const Elem * elem, const unsigned int side, const unsigned int state);

  /// The maximum state (0 = current, 1 = old, 2 = older)
  unsigned int _max_state;

  // You'd think a private mutex would work here, so I'll leave this
  // in the namespace for when that happens, but CI thinks that can
  // make us crash, so I'll just initialize this to Threads::spin_mtx
  libMesh::Threads::spin_mutex & _spin_mtx;

  /// Shared registry (across storage objects) for property names and IDs
  MaterialPropertyRegistry & _registry;

  /// The threaded material data
  std::vector<MaterialData> _material_data;

  typedef std::unordered_map<std::pair<const Elem *, unsigned int>,
                             std::map<unsigned int, std::vector<std::stringstream>>>
      RestartableMapType;

  /// The restartable data to be loaded in initStatefulProps() later
  RestartableMapType _restartable_map;

  /// Whether or not we want to restart stateful properties in place
  bool _restart_in_place;
  /// Whether or not we're recovering; enforces a one-to-one mapping of stateful properties
  bool _recovering;

  // Need to be able to eraseProperty from here
  friend class ProjectMaterialProperties;

  // Need to be able to initProps from here
  friend class RedistributeProperties;

  // Need non-const props from here
  friend void dataLoad(std::istream &, MaterialPropertyStorage &, void *);
  friend void dataStore(std::ostream &, MaterialPropertyStorage &, void *);
};

inline const MaterialPropertyStorage::PropsType &
MaterialPropertyStorage::props(const unsigned int state) const
{
  mooseAssert(state < _storage.size(), "Invalid material property state " + std::to_string(state));
  return _storage[state];
}

inline const MaterialProperties &
MaterialPropertyStorage::props(const Elem * elem, unsigned int side, const unsigned int state) const
{
  const auto find_elem = props(state).find(elem);
  mooseAssert(find_elem != props(state).end(), "Material property does not have elem entry");
  const auto find_side = find_elem->second.find(side);
  mooseAssert(find_side != find_elem->second.end(), "Material property does not have side entry");
  return find_side->second;
}

inline MaterialProperties &
MaterialPropertyStorage::setProps(const Elem * elem, unsigned int side, const unsigned int state)
{
  return const_cast<MaterialProperties &>(std::as_const(*this).props(elem, side, state));
}

inline MaterialProperties &
MaterialPropertyStorage::initAndSetProps(const Elem * elem,
                                         unsigned int side,
                                         const unsigned int state)
{
  return setProps(state)[elem][side];
}

inline MaterialPropertyStorage::PropsType &
MaterialPropertyStorage::setProps(const unsigned int state)
{
  return const_cast<MaterialPropertyStorage::PropsType &>(std::as_const(*this).props(state));
}

void dataStore(std::ostream & stream, MaterialPropertyStorage::PropRecord & record, void * context);
void dataLoad(std::istream & stream, MaterialPropertyStorage::PropRecord & record, void * context);
