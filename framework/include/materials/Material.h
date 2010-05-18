#ifndef MATERIAL_H
#define MATERIAL_H

#include "QpData.h"

//libmesh includes
#include "tensor_value.h"
#include "quadrature_gauss.h"
#include "ColumnMajorMatrix.h"

//forward declarations
class Material;
class MooseSystem;
class ElementData;

template<>
InputParameters validParams<Material>();

/**
 * Holds material properties that are assigned to blocks.
 */
class Material
{
public:

  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  Material(std::string name, MooseSystem & moose_system, InputParameters parameters);
  
  virtual ~Material()
  {
    // TODO: Implement destructor to clean up after the _qp_prev and _qp_curr data objects
    
    //std::for_each(_qp_prev.begin(), _qp_prev.end(), DeleteFunctor());
    //std::for_each(_qp_curr.begin(), _qp_curr.end(), DeleteFunctor());
  }

  /** 
   * Block ID the Material is active on.
   * 
   * @return The block ID.
   */
//  unsigned int blockID();

  /**
   * Causes the material to recompute all of it's values
   * at the quadrature points.  This is a helper in the base
   * class that does a bunch of common setup first then calls
   * computeProperties().
   */
  void materialReinit();

  /**
   * Retrieve the Constant Real valued property named "name"
   */
  Real & getConstantRealProperty(const std::string & name);
  
  /**
   * Retrieve the Real valued property named "name"
   */
  std::vector<Real> & getRealProperty(const std::string & name);
  
  /**
   * Retrieve the Gradient valued property named "name"
   */
  std::vector<RealGradient> & getGradientProperty(const std::string & name);

  /**
   * Retrieve RealVectorValue valued property named "name"
   */
  std::vector<RealVectorValue> & getRealVectorValueProperty(const std::string & name);

  /**
   * Retrieve the Vector valued property named "name"
   */
  std::vector<std::vector<Real> > & getVectorProperty(const std::string & name);
  
  /**
   * Retrieve the Tensor valued property named "name"
   */
  std::vector<RealTensorValue> & getTensorProperty(const std::string & name);
  
  /**
   * Retrieve the Tensor valued property named "name"
   */
  std::vector<ColumnMajorMatrix> & getColumnMajorMatrixProperty(const std::string & name);
  
  
  /**
   * Retrieve the Matrix valued property named "name"
   */
  std::vector<std::vector<std::vector<Real> > > & getMatrixProperty(const std::string & name);
  
  /**
   * Updates the old (first) material properties to the current/new material properies (second)
   */
  void updateDataState();

  /**
   * This virtual gets called every time the subdomain changes.  This is useful for doing pre-calcualtions
   * that should only be done once per subdomain.  In particular this is where references to material
   * property vectors should be initialized.
   */
  virtual void subdomainSetup();

protected:

  /**
   * This Material's name.
   */
  std::string _name;

  /**
   * Reference to the MooseSystem that this Kernel is associated to
   */
  MooseSystem & _moose_system;

  /**
   * Convenience reference to the ElementData object inside of MooseSystem
   */
  ElementData & _element_data;
  
  /**
   * The thread id this kernel is associated with.
   */
  THREAD_ID _tid;

  /**
   * Holds parameters for derived classes so they can be built with common constructor.
   */
  InputParameters _parameters;

  /**
   * The current dimension of the mesh.
   */
  unsigned int & _dim;

  /**
   * Current time.
   */
  Real & _t;

  /**
   * Current dt.
   */
  Real & _dt;

  /**
   * Old dt.
   */
  Real & _dt_old;

  /**
   * Whether or not the current simulation is transient.
   */
  bool & _is_transient;

  /**
   * Current element
   */
  const Elem * & _current_elem;

  /**
   * Current quadrature rule.
   */
  QGauss * & _qrule;

  /**
   * Variable numbers of the coupled variables.
   */
  std::vector<unsigned int> _coupled_var_nums;

  /**
   * Variable numbers of the coupled auxiliary variables.
   */
  std::vector<unsigned int> _aux_coupled_var_nums;

  /**
   * Names of the variables this kernel is coupled to.
   */
  std::vector<std::string> _coupled_to;

  /**
   * Names of the variables this kernel is coupled to.
   */
  std::vector<std::string> _coupled_as;

  /**
   * Map from _as_ to the actual variable number.
   */
  std::map<std::string, unsigned int> _coupled_as_to_var_num;

  /**
   * Map from _as_ to the actual variable number for auxiliary variables.
   */
  std::map<std::string, unsigned int> _aux_coupled_as_to_var_num;

  /**
   * Returns true if a variables has been coupled_as name.
   *
   * @param name The name the kernel wants to refer to the variable as.
   */
  bool isCoupled(std::string name);

  /**
   * Returns the variable number of the coupled variable.
   */
  unsigned int coupled(std::string name);

  /**
   * Returns a reference (that can be stored) to a coupled variable's value.
   *
   * @param name The name the kernel wants to refer to the variable as.
   */
  std::vector<Real> & coupledVal(std::string name);

  /**
   * Returns a reference (that can be stored) to a coupled variable's gradient.
   *
   * @param name The name the kernel wants to refer to the variable as.
   */
  std::vector<RealGradient> & coupledGrad(std::string name);

  /**
   * Just here for convenience.  Used in constructors... usually to deal with multiple dimensional stuff.
   */
  Real & _real_zero;
  std::vector<Real> & _zero;
  std::vector<RealGradient> & _grad_zero;
  std::vector<RealTensor> & _second_zero;

// struct DeleteFunctor 
//   {
//     void operator()(const std::pair<const unsigned int, std::vector<QpData *> > & p) const
//     {
//       //for(std::vector<QpData *>::iterator i = p.second.begin(); i != p.second.end(); ++i)
//       //  delete *i;
//       std::cerr << p.first << " ";
//       //std::vector<const QpData *>::iterator i = p.second;
//       std::cerr << p.second[0] << std::endl;
//     }
//  };
  
  enum QP_Data_Type { CURR, PREV };

  /**
   * All materials must override this virtual.
   * This is where they fill up the vectors with values.
   */
  virtual void computeProperties() = 0;

  /**
   * This function is called to create the data structure that will be associated
   * with a quadrature point
   */
  virtual QpData * createData();

  /**
   * This function returns a reference to a standard vector of datastructures for all 
   * the quadrature points on the current element
   */
  virtual std::vector<QpData *> & getData(QP_Data_Type qp_data_type); 

  /**
   * Block ID this material is active on.
   */
//  unsigned int _block_id;

  /**
   * Doesn't do anything for materials.
   */
  virtual Real computeQpResidual();

  /**
   * Declare the Real valued property named "name".
   * This must be done _before_ a property of that name is tried
   * to be retrieved using get().
   */
  std::vector<Real> & declareRealProperty(const std::string & name);
  
  /**
   * Declare the Constant Real valued property named "name".
   * This must be done _before_ a property of that name is tried
   * to be retrieved using get().
   */
  Real & declareConstantRealProperty(const std::string & name);
  
  /**
   * Declare the Gradient valued property named "name".
   * This must be done _before_ a property of that name is tried
   * to be retrieved using get().
   */
  std::vector<RealGradient> & declareGradientProperty(const std::string & name);

  /**
   * Declare the RealVectorValue valued property named "name".
   * This must be done _before_ a property of that name is tried
   * to be retrieved using get().
   */
  std::vector<RealVectorValue> & declareRealVectorValueProperty(const std::string & name);

  /**
   * Declare the Vector valued property named "name".
   * This must be done _before_ a property of that name is tried
   * to be retrieved using get().
   */
  std::vector<std::vector<Real> > & declareVectorProperty(const std::string & name);
  
  /**
   * Declare the Tensor valued property named "name".
   * This must be done _before_ a property of that name is tried
   * to be retrieved using get().
   */
  std::vector<RealTensorValue> & declareTensorProperty(const std::string & name);
  
  /**
   * Declare the Tensor valued property named "name".
   * This must be done _before_ a property of that name is tried
   * to be retrieved using get().
   */
  std::vector<ColumnMajorMatrix> & declareColumnMajorMatrixProperty(const std::string & name);
  
  /**
   * Declare the Matrix valued property named "name".
   * This must be done _before_ a property of that name is tried
   * to be retrieved using get().
   */
  std::vector<std::vector<std::vector<Real> > > & declareMatrixProperty(const std::string & name);
  
  std::map<std::string, Real > _constant_real_props;
  std::map<std::string, std::vector<Real> > _real_props;
  std::map<std::string, std::vector<RealGradient> > _gradient_props;
  std::map<std::string, std::vector<RealVectorValue> > _real_vector_value_props;
  std::map<std::string, std::vector<std::vector<Real> > > _vector_props;
  std::map<std::string, std::vector<RealTensorValue> > _tensor_props;
  std::map<std::string, std::vector<ColumnMajorMatrix> > _column_major_matrix_props;
  std::map<std::string, std::vector<std::vector<std::vector<Real> > > > _matrix_props;
  std::map<unsigned int, std::vector<QpData *> > _qp_prev;
  std::map<unsigned int, std::vector<QpData *> > _qp_curr;

//  QGauss * _qrule;
  
//private:
//  Kernel::_qrule;

  /**
   * Whether or not this coupled_as name is associated with an auxiliary variable.
   */
  bool isAux(std::string name);
};

#endif //MATERIAL_H
