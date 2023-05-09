//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOOSE includes
#include "MooseObject.h"
#include "BlockRestrictable.h"
#include "BoundaryRestrictable.h"
#include "SetupInterface.h"
#include "MooseVariableDependencyInterface.h"
#include "ScalarCoupleable.h"
#include "FunctionInterface.h"
#include "DistributionInterface.h"
#include "UserObjectInterface.h"
#include "TransientInterface.h"
#include "PostprocessorInterface.h"
#include "VectorPostprocessorInterface.h"
#include "DependencyResolverInterface.h"
#include "Restartable.h"
#include "MeshChangedInterface.h"
#include "OutputInterface.h"
#include "RandomInterface.h"
#include "ElementIDInterface.h"
#include "MaterialProperty.h"
#include "MaterialData.h"
#include "MathUtils.h"
#include "Assembly.h"
#include "GeometricSearchInterface.h"
#include "ADFunctorInterface.h"
#include "SolutionInvalidInterface.h"

#define usingMaterialBaseMembers                                                                   \
  usingTransientInterfaceMembers;                                                                  \
  using MaterialBase::_subproblem;                                                                 \
  using MaterialBase::_fe_problem;                                                                 \
  using MaterialBase::_tid;                                                                        \
  using MaterialBase::_assembly;                                                                   \
  using MaterialBase::_qp;                                                                         \
  using MaterialBase::_coord;                                                                      \
  using MaterialBase::_normals;                                                                    \
  using MaterialBase::_mesh

// forward declarations
class MaterialBase;
class MooseMesh;
class SubProblem;
class FaceInfo;

/**
 * MaterialBases compute MaterialProperties.
 */
class MaterialBase : public MooseObject,
                     public BlockRestrictable,
                     public BoundaryRestrictable,
                     public SetupInterface,
                     public MooseVariableDependencyInterface,
                     public ScalarCoupleable,
                     public FunctionInterface,
                     public DistributionInterface,
                     public UserObjectInterface,
                     public TransientInterface,
                     public PostprocessorInterface,
                     public VectorPostprocessorInterface,
                     public DependencyResolverInterface,
                     public Restartable,
                     public MeshChangedInterface,
                     public OutputInterface,
                     public RandomInterface,
                     public ElementIDInterface,
                     protected GeometricSearchInterface,
                     protected ADFunctorInterface,
                     protected SolutionInvalidInterface
{
public:
  static InputParameters validParams();

  MaterialBase(const InputParameters & parameters);

  /**
   * Initialize stateful properties (if material has some)
   */
  virtual void initStatefulProperties(unsigned int n_points);
  virtual bool isInterfaceMaterial() { return false; };

  /**
   * Performs the quadrature point loop, calling computeQpProperties
   */
  virtual void computeProperties() = 0;

  /**
   * Resets the properties at each quadrature point (see resetQpProperties), only called if 'compute
   * = false'.
   *
   * This method is called internally by MOOSE, you probably don't want to mess with this.
   */
  virtual void resetProperties();

  /**
   * A method for (re)computing the properties of a MaterialBase.
   *
   * This is intended to be called from other objects, by first calling
   * MaterialPropertyInterface::getMaterial and then calling this method on the MaterialBase object
   * returned.
   */
  virtual void computePropertiesAtQp(unsigned int qp);

  ///@{
  /**
   * Declare the property named "name"
   */
  template <typename T>
  MaterialProperty<T> & declarePropertyByName(const std::string & prop_name);
  template <typename T>
  MaterialProperty<T> & declareProperty(const std::string & name);
  template <typename T>
  MaterialProperty<T> & declarePropertyOld(const std::string & prop_name);
  template <typename T>
  MaterialProperty<T> & declarePropertyOlder(const std::string & prop_name);
  template <typename T>
  ADMaterialProperty<T> & declareADPropertyByName(const std::string & prop_name);
  template <typename T>
  ADMaterialProperty<T> & declareADProperty(const std::string & name);

  template <typename T, bool is_ad>
  auto & declareGenericProperty(const std::string & prop_name)
  {
    if constexpr (is_ad)
      return declareADProperty<T>(prop_name);
    else
      return declareProperty<T>(prop_name);
  }
  template <typename T, bool is_ad>
  auto & declareGenericPropertyByName(const std::string & prop_name)
  {
    if constexpr (is_ad)
      return declareADPropertyByName<T>(prop_name);
    else
      return declarePropertyByName<T>(prop_name);
  }
  ///@}

  /**
   * Return a material property that is initialized to zero by default and does
   * not need to (but can) be declared by another material.
   */
  template <typename T, bool is_ad>
  const GenericMaterialProperty<T, is_ad> &
  getGenericZeroMaterialProperty(const std::string & name);
  template <typename T, bool is_ad>
  const GenericMaterialProperty<T, is_ad> &
  getGenericZeroMaterialPropertyByName(const std::string & prop_name);

  /**
   * Return a constant zero anonymous material property
   */
  template <typename T, bool is_ad>
  const GenericMaterialProperty<T, is_ad> & getGenericZeroMaterialProperty();

  /// for backwards compatibility
  template <typename T, typename... Ts>
  const MaterialProperty<T> & getZeroMaterialProperty(Ts... args)
  {
    return getGenericZeroMaterialProperty<T, false>(args...);
  }

  /// for backwards compatibility
  template <typename T, typename... Ts>
  const MaterialProperty<T> & getZeroMaterialPropertyByName(Ts... args)
  {
    return getGenericZeroMaterialPropertyByName<T, false>(args...);
  }

  /**
   * Return a set of properties accessed with getMaterialProperty
   * @return A reference to the set of properties with calls to getMaterialProperty
   */
  virtual const std::set<std::string> & getRequestedItems() override { return _requested_props; }

  /**
   * Return a set of properties accessed with declareProperty
   * @return A reference to the set of properties with calls to declareProperty
   */
  virtual const std::set<std::string> & getSuppliedItems() override { return _supplied_props; }

  /**
   * Get the prop ids corresponding to \p declareProperty
   * @return A reference to the set of properties with calls to \p declareProperty
   */
  const std::set<unsigned int> & getSuppliedPropIDs() { return _supplied_prop_ids; }

  void checkStatefulSanity() const;

  /**
   * Get the list of output objects that this class is restricted
   * @return A vector of OutputNames
   */
  std::set<OutputName> getOutputs();

  /**
   * Returns true of the MaterialData type is not associated with volume data
   */
  virtual bool isBoundaryMaterial() const = 0;

  /**
   * Subdomain setup evaluating material properties when required
   */
  virtual void subdomainSetup() override;

  /**
   * Retrieve the set of material properties that _this_ object depends on.
   *
   * @return The IDs corresponding to the material properties that
   * MUST be reinited before evaluating this object
   */
  virtual const std::set<unsigned int> & getMatPropDependencies() const = 0;

  /**
   * @return Whether this material has stateful properties
   */
  bool hasStatefulProperties() const { return _has_stateful_property; }

  /**
   * Whether this material supports ghosted computations. This is important for finite volume
   * calculations in which variables have defined values on ghost cells/elements and for which these
   * ghost values may need to flow through material calculations to be eventually consumed by FV
   * flux kernels or boundary conditions
   */
  virtual bool ghostable() const { return false; }

  void setFaceInfo(const FaceInfo & fi) { _face_info = &fi; }

  /**
   * Build the materials required by a set of consumer objects
   */
  template <typename Consumers>
  static std::deque<MaterialBase *>
  buildRequiredMaterials(const Consumers & mat_consumers,
                         const std::vector<std::shared_ptr<MaterialBase>> & mats,
                         const bool allow_stateful);

protected:
  /**
   * Users must override this method.
   */
  virtual void computeQpProperties();

  /**
   * Resets the properties prior to calculation of traditional materials (only if 'compute =
   * false').
   *
   * This method must be overridden in your class. This is called just prior to the re-calculation
   * of
   * traditional material properties to ensure that the properties are in a proper state for
   * calculation.
   */
  virtual void resetQpProperties();

  /**
   * Initialize stateful properties at quadrature points.  Note when using this function you only
   * need to address
   * the "current" material properties not the old ones directly, i.e. if you have a property named
   * "_diffusivity"
   * and an older property named "_diffusivity_old".  You only need to initialize diffusivity.
   * MOOSE will use
   * copy that initial value to the old and older values as necessary.
   */
  virtual void initQpStatefulProperties();

  virtual const MaterialData & materialData() const = 0;
  virtual MaterialData & materialData() = 0;

  virtual const FEProblemBase & miProblem() const { return _fe_problem; }
  virtual FEProblemBase & miProblem() { return _fe_problem; }

  virtual const QBase & qRule() const = 0;

  SubProblem & _subproblem;

  FEProblemBase & _fe_problem;
  THREAD_ID _tid;
  Assembly & _assembly;

  unsigned int _qp;

  const MooseArray<Real> & _coord;
  /// normals at quadrature points (valid only in boundary materials)
  const MooseArray<Point> & _normals;

  MooseMesh & _mesh;

  /// Coordinate system
  const Moose::CoordinateSystemType & _coord_sys;

  /// Set of properties accessed via get method
  std::set<std::string> _requested_props;

  /// Set of properties declared
  std::set<std::string> _supplied_props;

  /// The ids of the supplied properties, i.e. the indices where they
  /// are stored in the _material_data->props().  Note: these ids ARE
  /// NOT IN THE SAME ORDER AS THE _supplied_props set, which is
  /// ordered alphabetically by name.  The intention of this container
  /// is to allow rapid copying of MaterialProperty values in
  /// MaterialBase::computeProperties() without looking up the ids from
  /// the name strings each time.
  std::set<unsigned int> _supplied_prop_ids;

  /// If False MOOSE does not compute this property
  const bool _compute;

  enum QP_Data_Type
  {
    CURR,
    PREV
  };

  std::map<std::string, MaterialPropStateInt> _props_to_flags;

  /// Small helper function to call store{Subdomain,Boundary}MatPropName
  void registerPropName(std::string prop_name, bool is_get, MaterialPropState state);

  /// Check and throw an error if the execution has progerssed past the construction stage
  void checkExecutionStage();

  std::vector<unsigned int> _displacements;

  // private:
  bool _has_stateful_property;

  bool _overrides_init_stateful_props = true;

  const FaceInfo * _face_info = nullptr;

private:
  const MaterialPropertyName _declare_suffix;
};

template <typename T>
MaterialProperty<T> &
MaterialBase::declareProperty(const std::string & name)
{
  // Check if the supplied parameter is a valid input parameter key
  std::string prop_name = name;
  if (_pars.have_parameter<MaterialPropertyName>(name))
    prop_name = _pars.get<MaterialPropertyName>(name);

  return declarePropertyByName<T>(prop_name);
}

template <typename T>
MaterialProperty<T> &
MaterialBase::declarePropertyByName(const std::string & prop_name_in)
{
  const auto prop_name =
      _declare_suffix.empty()
          ? prop_name_in
          : MooseUtils::join(std::vector<std::string>({prop_name_in, _declare_suffix}), "_");
  registerPropName(prop_name, false, MaterialPropState::CURRENT);
  return materialData().declareProperty<T>(prop_name);
}

template <typename T>
MaterialProperty<T> &
MaterialBase::declarePropertyOld(const std::string & prop_name)
{
  mooseDoOnce(
      mooseDeprecated("declarePropertyOld is deprecated and not needed anymore.\nUse "
                      "getMaterialPropertyOld (only) if a reference is required in this class."));
  registerPropName(prop_name, false, MaterialPropState::OLD);
  return materialData().declarePropertyOld<T>(prop_name);
}

template <typename T>
MaterialProperty<T> &
MaterialBase::declarePropertyOlder(const std::string & prop_name)
{
  mooseDoOnce(
      mooseDeprecated("declarePropertyOlder is deprecated and not needed anymore.  Use "
                      "getMaterialPropertyOlder (only) if a reference is required in this class."));
  registerPropName(prop_name, false, MaterialPropState::OLDER);
  return materialData().declarePropertyOlder<T>(prop_name);
}

template <typename T, bool is_ad>
const GenericMaterialProperty<T, is_ad> &
MaterialBase::getGenericZeroMaterialProperty(const std::string & name)
{
  // Check if the supplied parameter is a valid input parameter key
  std::string prop_name = name;
  if (_pars.have_parameter<MaterialPropertyName>(name))
    prop_name = _pars.get<MaterialPropertyName>(name);

  return getGenericZeroMaterialPropertyByName<T, is_ad>(prop_name);
}

template <typename T, bool is_ad>
const GenericMaterialProperty<T, is_ad> &
MaterialBase::getGenericZeroMaterialPropertyByName(const std::string & prop_name)
{
  checkExecutionStage();
  auto & preload_with_zero = materialData().getGenericProperty<T, is_ad>(prop_name);

  _requested_props.insert(prop_name);
  registerPropName(prop_name, true, MaterialPropState::CURRENT);
  _fe_problem.markMatPropRequested(prop_name);

  // Register this material on these blocks and boundaries as a zero property with relaxed
  // consistency checking
  for (std::set<SubdomainID>::const_iterator it = blockIDs().begin(); it != blockIDs().end(); ++it)
    _fe_problem.storeSubdomainZeroMatProp(*it, prop_name);
  for (std::set<BoundaryID>::const_iterator it = boundaryIDs().begin(); it != boundaryIDs().end();
       ++it)
    _fe_problem.storeBoundaryZeroMatProp(*it, prop_name);

  // set values for all qpoints to zero
  // (in multiapp scenarios getMaxQps can return different values in each app; we need the max)
  unsigned int nqp = _fe_problem.getMaxQps();
  if (nqp > preload_with_zero.size())
    preload_with_zero.resize(nqp);
  for (unsigned int qp = 0; qp < nqp; ++qp)
    MathUtils::mooseSetToZero(preload_with_zero[qp]);

  return preload_with_zero;
}

template <typename T, bool is_ad>
const GenericMaterialProperty<T, is_ad> &
MaterialBase::getGenericZeroMaterialProperty()
{
  // static zero property storage
  static GenericMaterialProperty<T, is_ad> zero;

  // resize to accomodate maximum number of qpoints
  // (in multiapp scenarios getMaxQps can return different values in each app; we need the max)
  unsigned int nqp = _fe_problem.getMaxQps();
  if (nqp > zero.size())
    zero.resize(nqp);

  // set values for all qpoints to zero
  for (unsigned int qp = 0; qp < nqp; ++qp)
    MathUtils::mooseSetToZero(zero[qp]);

  return zero;
}

template <typename T>
ADMaterialProperty<T> &
MaterialBase::declareADProperty(const std::string & name)
{
  // Check if the supplied parameter is a valid input parameter key
  std::string prop_name = name;
  if (_pars.have_parameter<MaterialPropertyName>(name))
    prop_name = _pars.get<MaterialPropertyName>(name);

  return declareADPropertyByName<T>(prop_name);
}

template <typename T>
ADMaterialProperty<T> &
MaterialBase::declareADPropertyByName(const std::string & prop_name_in)
{
  const auto prop_name =
      _declare_suffix.empty()
          ? prop_name_in
          : MooseUtils::join(std::vector<std::string>({prop_name_in, _declare_suffix}), "_");
  registerPropName(prop_name, false, MaterialPropState::CURRENT);
  return materialData().declareADProperty<T>(prop_name);
}

template <typename Consumers>
std::deque<MaterialBase *>
MaterialBase::buildRequiredMaterials(const Consumers & mat_consumers,
                                     const std::vector<std::shared_ptr<MaterialBase>> & mats,
                                     const bool allow_stateful)
{
  std::deque<MaterialBase *> required_mats;

  std::unordered_set<unsigned int> needed_mat_props;
  for (const auto & consumer : mat_consumers)
  {
    const auto & mp_deps = consumer->getMatPropDependencies();
    needed_mat_props.insert(mp_deps.begin(), mp_deps.end());
  }

  // A predicate of calling this function is that these materials come in already sorted by
  // dependency with the front of the container having no other material dependencies and following
  // materials potentially depending on the ones in front of them. So we can start at the back and
  // iterate forward checking whether the current material supplies anything that is needed, and if
  // not we discard it
  for (auto it = mats.rbegin(); it != mats.rend(); ++it)
  {
    auto * const mat = it->get();
    bool supplies_needed = false;

    const auto & supplied_props = mat->getSuppliedPropIDs();

    // Do O(N) with the small container
    for (const auto supplied_prop : supplied_props)
    {
      if (needed_mat_props.count(supplied_prop))
      {
        supplies_needed = true;
        break;
      }
    }

    if (!supplies_needed)
      continue;

    if (!allow_stateful && mat->hasStatefulProperties())
      ::mooseError(
          "Someone called buildRequiredMaterials with allow_stateful = false but a material "
          "dependency ",
          mat->name(),
          " computes stateful properties.");

    const auto & mp_deps = mat->getMatPropDependencies();
    needed_mat_props.insert(mp_deps.begin(), mp_deps.end());
    required_mats.push_front(mat);
  }

  return required_mats;
}
