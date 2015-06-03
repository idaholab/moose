/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef MATERIAL_H
#define MATERIAL_H

#include "MooseObject.h"
#include "SetupInterface.h"
#include "Coupleable.h"
#include "ScalarCoupleable.h"
#include "MooseVariableDependencyInterface.h"
#include "FunctionInterface.h"
#include "UserObjectInterface.h"
#include "TransientInterface.h"
#include "PostprocessorInterface.h"
#include "MaterialProperty.h"
#include "MaterialPropertyInterface.h"
#include "MaterialData.h"
#include "ParallelUniqueId.h"
#include "Problem.h"
#include "SubProblem.h"
#include "DependencyResolverInterface.h"
#include "Function.h"
#include "BlockRestrictable.h"
#include "BoundaryRestrictable.h"
#include "Restartable.h"
#include "ZeroInterface.h"
#include "MeshChangedInterface.h"
#include "OutputInterface.h"

// libMesh includes
#include "libmesh/quadrature_gauss.h"
#include "libmesh/elem.h"

// forward declarations
class Material;
class MooseMesh;

/**
 * Holds a data structure used to compute material properties at a Quadrature point
 */
struct QpData
{
  virtual ~QpData(){}

  inline virtual QpData& operator=(const QpData &) { return *this; }
};


template<>
InputParameters validParams<Material>();

/**
 * Holds material properties that are assigned to blocks.
 */
class Material :
  public MooseObject,
  public BlockRestrictable,
  public BoundaryRestrictable,
  public SetupInterface,
  public Coupleable,
  public MooseVariableDependencyInterface,
  public ScalarCoupleable,
  public FunctionInterface,
  public UserObjectInterface,
  public TransientInterface,
  public MaterialPropertyInterface,
  public PostprocessorInterface,
  public DependencyResolverInterface,
  public Restartable,
  public ZeroInterface,
  public MeshChangedInterface,
  public OutputInterface
{
public:
  Material(const std::string & name, InputParameters parameters);

  virtual ~Material();

  /**
   * Compute the all the properties at all for the quadrature points.
   *
   * By default this loops over the quadrature points and calls computeQpProperties
   */
  virtual void computeProperties();

  /**
   * Initialize stateful properties (if material has some)
   */
  virtual void initStatefulProperties(unsigned int n_points);

  ///@{
  /**
   * Retrieve the property named "name"
   */
  template<typename T>
  MaterialProperty<T> & getMaterialProperty(const std::string & prop_name);
  template<typename T>
  MaterialProperty<T> & getMaterialPropertyOld(const std::string & prop_name);
  template<typename T>
  MaterialProperty<T> & getMaterialPropertyOlder(const std::string & prop_name);
  ///@}

  ///@{
  /**
   * Declare the property named "name"
   */
  template<typename T>
  MaterialProperty<T> & declareProperty(const std::string & prop_name);
  template<typename T>
  MaterialProperty<T> & declarePropertyOld(const std::string & prop_name);
  template<typename T>
  MaterialProperty<T> & declarePropertyOlder(const std::string & prop_name);
  ///@}

  /**
   * Get the material property id
   * @param prop_name The name of the property to which you want the id
   */
   template<typename T>
   unsigned int materialProperty(const std::string & prop_name);

  /**
   * Return a set of properties accessed with getMaterialProperty
   * @return A reference to the set of properties with calls to getMaterialProperty
   *
   * This method is used to test for cyclic dependencies, which in the case
   * of properties that are being recomputed is different from the set
   * provided by getDependItems. The set returned by getRequestedItems does
   * not include properties that have calls to 'materialProperty'.
   */
  virtual
  const std::set<std::string> &
  getRequestedItems() { return _requested_props; }

  /**
   * Return a set of properties for dependency resolution
   * @return The set of property names with calls to materialProperty
   *
   * @see getRequestedItems
   */
  virtual
  const std::set<std::string> &
  getIterativeItems(){ return _iterative_props; }

  /**
   * Return a set of properties accessed with declareProperty
   * @return A reference to the set of properties with calls to declareProperty
   */
  virtual
  const std::set<std::string> &
  getSuppliedItems() { return _supplied_props; }

  void checkStatefulSanity() const;

  /**
   * Check if a material property is valid for all blocks of this Material
   *
   * This method returns true if the supplied property name has been declared
   * in a Material object on the block ids for this object.
   *
   * @param prop_name the name of the property to query
   * @return true if the property exists for all block ids of the object, otherwise false
   *
   * @see BlockRestrictable::hasBlockMaterialPropertyHelper
   */
  virtual bool hasBlockMaterialPropertyHelper(const std::string & prop_name);

  /**
   * Get the list of output objects that this class is restricted
   * @return A vector of OutputNames
   */
  std::set<OutputName> getOutputs();

protected:

  /// Reference to the SubProblem
  SubProblem & _subproblem;

  /// Reference to the FEProblem object
  FEProblem & _fe_problem;

  /// Objects thread id
  THREAD_ID _tid;

  /// Assembly object
  Assembly & _assembly;

  /// Flag indicating that the Material object is a Boundary material
  bool _bnd;

  /// Flag indicating that the Material is a Neighbor material
  bool _neighbor;

  /// Reference to the correct MaterialData object
  MaterialData & _material_data;

  /// The quadrature point index
  unsigned int _qp;

  ///@{
  /// Current quadrature rule and other FE items
  QBase * & _qrule;
  const MooseArray<Real> & _JxW;
  const MooseArray<Real> & _coord;
  const MooseArray<Point> & _q_point;
  const MooseArray<Point> & _normals;
  const Elem * & _current_elem;
  unsigned int & _current_side;
  ///@}

  /// Reference to the Mesh
  MooseMesh & _mesh;

  /// Coordinate system
  const Moose::CoordinateSystemType & _coord_sys;

  /// Set of properties accessed via get method
  std::set<std::string> _iterative_props;

  /// Set of properties accessed via get method and not the 'materialProperty' method
  std::set<std::string> _requested_props;

  /// Set of properties declared
  std::set<std::string> _supplied_props;

  enum QP_Data_Type {
    CURR,
    PREV
  };

  enum Prop_State
  {
    CURRENT = 0x1,
    OLD = 0x2,
    OLDER = 0x4
  };
  std::map<std::string, int> _props_to_flags;

  /**
   * Initialize stateful properties at quadrature points.  Note when using this function you only need to address
   * the "current" material properties not the old ones directly, i.e. if you have a property named "_diffusivity"
   * and an older property named "_diffusivity_old".  You only need to initialize diffusivity.  MOOSE will use
   * copy that initial value to the old and older values as necessary.
   */
  virtual void initQpStatefulProperties();

  /**
   * Compute material properties in quadrature point
   * Materials has to redefine this.
   */
  virtual void computeQpProperties();

  /**
   * This function is called to create the data structure that will be associated
   * with a quadrature point
   */
  virtual QpData * createData();

  ///@{
  /**
   * Recalculate the specified material object given a property id
   * @param prop_id The property id(s) of interest that needs to be recomputed, this id should be retrieved with
   *                a call to `materialProperty`.
   * @param qp The quadrature point index at which to recompute the properties
   *
   * Note, this method calls computeQpProperties on the Material object responsible for computing the supplied property;
   * hence, this will also compute all other properties as well.
   */
  void recomputeMaterial(const unsigned int & prop_id, unsigned int qp);
  void recomputeMaterial(const std::vector<unsigned int> & prop_id, unsigned int qp);
  ///@}

  ///@{
  /// Maps to quadrature point information
  std::map<unsigned int, std::vector<QpData *> > _qp_prev;
  std::map<unsigned int, std::vector<QpData *> > _qp_curr;
  ///@}

private:

  /**
   * A helper function for recomputing the material properties at a specific quadrature point
   * @param qp The quadrature point to recompute
   */
  void computeProperties(unsigned int qp);

  /**
   * Helper functions to call storeMatPropName
   */
  void registerPropName(const std::string & prop_name);
  void registerSuppliedPropName(const std::string & prop_name, Prop_State state);

  /// Reference to the MaterialWarehouse
  MaterialWarehouse & _material_warehouse;

  /// True when the material object contains stateful properties
  bool _has_stateful_property;
};

template<typename T>
unsigned int
Material::materialProperty(const std::string & prop_name)
{
  _iterative_props.insert(prop_name);
  registerPropName(prop_name);
  _fe_problem.markMatPropRequested(prop_name);
  return _material_data.getPropertyId(prop_name);
}

template<typename T>
MaterialProperty<T> &
Material::getMaterialProperty(const std::string & prop_name)
{
  _requested_props.insert(prop_name);
  registerPropName(prop_name);
  _fe_problem.markMatPropRequested(prop_name);
  return _material_data.getProperty<T>(prop_name);
}

template<typename T>
MaterialProperty<T> &
Material::getMaterialPropertyOld(const std::string & prop_name)
{
  registerPropName(prop_name);
  _fe_problem.markMatPropRequested(prop_name);
  return _material_data.getPropertyOld<T>(prop_name);
}

template<typename T>
MaterialProperty<T> &
Material::getMaterialPropertyOlder(const std::string & prop_name)
{
  registerPropName(prop_name);
  _fe_problem.markMatPropRequested(prop_name);
  return _material_data.getPropertyOlder<T>(prop_name);
}

template<typename T>
MaterialProperty<T> &
Material::declareProperty(const std::string & prop_name)
{
  _supplied_props.insert(prop_name);
  registerSuppliedPropName(prop_name, Material::CURRENT);
  return _material_data.declareProperty<T>(prop_name);
}

template<typename T>
MaterialProperty<T> &
Material::declarePropertyOld(const std::string & prop_name)
{
  registerSuppliedPropName(prop_name, Material::OLD);
  return _material_data.declarePropertyOld<T>(prop_name);
}

template<typename T>
MaterialProperty<T> &
Material::declarePropertyOlder(const std::string & prop_name)
{
  registerSuppliedPropName(prop_name, Material::OLDER);
  return _material_data.declarePropertyOlder<T>(prop_name);
}

#endif //MATERIAL_H
