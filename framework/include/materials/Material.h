#ifndef MATERIAL_H
#define MATERIAL_H


#include "MooseArray.h"
#include "PDEBase.h"
#include "QuadraturePointData.h"
#include "ColumnMajorMatrix.h"

//forward declarations
class Material;
class MooseSystem;
class ElementData;
class MaterialData;
class QpData;

template<>
InputParameters validParams<Material>();

/**
 * Holds material properties that are assigned to blocks.
 */
class Material :
  public PDEBase
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

  bool hasStatefulProperties();
  
  /**
   * Retrieve the Constant Real valued property named "name"
   */
  Real & getConstantRealProperty(const std::string & name);
  
  /**
   * Retrieve the Real valued property named "name"
   */
  MooseArray<Real> & getRealProperty(const std::string & name);
  
  /**
   * Retrieve the Gradient valued property named "name"
   */
  MooseArray<RealGradient> & getGradientProperty(const std::string & name);

  /**
   * Retrieve RealVectorValue valued property named "name"
   */
  MooseArray<RealVectorValue> & getRealVectorValueProperty(const std::string & name);

  /**
   * Retrieve the Vector valued property named "name"
   */
  MooseArray<MooseArray<Real> > & getVectorProperty(const std::string & name);
  
  /**
   * Retrieve the Tensor valued property named "name"
   */
  MooseArray<RealTensorValue> & getTensorProperty(const std::string & name);
  
  /**
   * Retrieve the Tensor valued property named "name"
   */
  MooseArray<ColumnMajorMatrix> & getColumnMajorMatrixProperty(const std::string & name);  
  
  /**
   * Retrieve the Matrix valued property named "name"
   */
  MooseArray<MooseArray<MooseArray<Real> > > & getMatrixProperty(const std::string & name);



  /**
   * Retrieve the Constant Real valued property named "name"
   */
  Real & getConstantRealPropertyOld(const std::string & name);
  
  /**
   * Retrieve the Real valued property named "name"
   */
  MooseArray<Real> & getRealPropertyOld(const std::string & name);
  
  /**
   * Retrieve the Gradient valued property named "name"
   */
  MooseArray<RealGradient> & getGradientPropertyOld(const std::string & name);

  /**
   * Retrieve RealVectorValue valued property named "name"
   */
  MooseArray<RealVectorValue> & getRealVectorValuePropertyOld(const std::string & name);

  /**
   * Retrieve the Vector valued property named "name"
   */
  MooseArray<MooseArray<Real> > & getVectorPropertyOld(const std::string & name);
  
  /**
   * Retrieve the Tensor valued property named "name"
   */
  MooseArray<RealTensorValue> & getTensorPropertyOld(const std::string & name);
  
  /**
   * Retrieve the Tensor valued property named "name"
   */
  MooseArray<ColumnMajorMatrix> & getColumnMajorMatrixPropertyOld(const std::string & name);  
  
  /**
   * Retrieve the Matrix valued property named "name"
   */
  MooseArray<MooseArray<MooseArray<Real> > > & getMatrixPropertyOld(const std::string & name);




  /**
   * Retrieve the Constant Real valued property named "name"
   */
  Real & getConstantRealPropertyOlder(const std::string & name);
  
  /**
   * Retrieve the Real valued property named "name"
   */
  MooseArray<Real> & getRealPropertyOlder(const std::string & name);
  
  /**
   * Retrieve the Gradient valued property named "name"
   */
  MooseArray<RealGradient> & getGradientPropertyOlder(const std::string & name);

  /**
   * Retrieve RealVectorValue valued property named "name"
   */
  MooseArray<RealVectorValue> & getRealVectorValuePropertyOlder(const std::string & name);

  /**
   * Retrieve the Vector valued property named "name"
   */
  MooseArray<MooseArray<Real> > & getVectorPropertyOlder(const std::string & name);
  
  /**
   * Retrieve the Tensor valued property named "name"
   */
  MooseArray<RealTensorValue> & getTensorPropertyOlder(const std::string & name);
  
  /**
   * Retrieve the Tensor valued property named "name"
   */
  MooseArray<ColumnMajorMatrix> & getColumnMajorMatrixPropertyOlder(const std::string & name);  
  
  /**
   * Retrieve the Matrix valued property named "name"
   */
  MooseArray<MooseArray<MooseArray<Real> > > & getMatrixPropertyOlder(const std::string & name);


  
  /**
   * Updates the old (first) material properties to the current/new material properies (second)
   */
  void updateDataState();

  /**
   * This function is called at the beginning of each timestep
   * for each active material block
   */
  virtual void timeStepSetup();

protected:

  /**
   * Convenience reference to the MaterialData object inside of MooseSystem
   */
  MaterialData & _material_data;
  
  /**
   * Whether or not this material has stateful properties.  This will get automatically
   * set to true if a stateful property is declared.
   */
  bool _has_stateful_props;

  /**
   * Current quadrature rule size
   */
  unsigned int _n_qpoints;

// struct DeleteFunctor 
//   {
//     void operator()(const std::pair<const unsigned int, MooseArray<QpData *> > & p) const
//     {
//       //for(MooseArray<QpData *>::iterator i = p.second.begin(); i != p.second.end(); ++i)
//       //  delete *i;
//       std::cerr << p.first << " ";
//       //MooseArray<const QpData *>::iterator i = p.second;
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
  MooseArray<Real> & declareRealProperty(const std::string & name);
  
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
  MooseArray<RealGradient> & declareGradientProperty(const std::string & name);

  /**
   * Declare the RealVectorValue valued property named "name".
   * This must be done _before_ a property of that name is tried
   * to be retrieved using get().
   */
  MooseArray<RealVectorValue> & declareRealVectorValueProperty(const std::string & name);

  /**
   * Declare the Vector valued property named "name".
   * This must be done _before_ a property of that name is tried
   * to be retrieved using get().
   */
  MooseArray<MooseArray<Real> > & declareVectorProperty(const std::string & name);
  
  /**
   * Declare the Tensor valued property named "name".
   * This must be done _before_ a property of that name is tried
   * to be retrieved using get().
   */
  MooseArray<RealTensorValue> & declareTensorProperty(const std::string & name);
  
  /**
   * Declare the Tensor valued property named "name".
   * This must be done _before_ a property of that name is tried
   * to be retrieved using get().
   */
  MooseArray<ColumnMajorMatrix> & declareColumnMajorMatrixProperty(const std::string & name);
  
  /**
   * Declare the Matrix valued property named "name".
   * This must be done _before_ a property of that name is tried
   * to be retrieved using get().
   */
  MooseArray<MooseArray<MooseArray<Real> > > & declareMatrixProperty(const std::string & name);


  /**
   * Declare the Real valued property named "name".
   * This must be done _before_ a property of that name is tried
   * to be retrieved using getOld().
   */
  MooseArray<Real> & declareRealPropertyOld(const std::string & name);
  
  /**
   * Declare the Constant Real valued property named "name".
   * This must be done _before_ a property of that name is tried
   * to be retrieved using getOld().
   */
  Real & declareConstantRealPropertyOld(const std::string & name);
  
  /**
   * Declare the Gradient valued property named "name".
   * This must be done _before_ a property of that name is tried
   * to be retrieved using getOld().
   */
  MooseArray<RealGradient> & declareGradientPropertyOld(const std::string & name);

  /**
   * Declare the RealVectorValue valued property named "name".
   * This must be done _before_ a property of that name is tried
   * to be retrieved using getOld().
   */
  MooseArray<RealVectorValue> & declareRealVectorValuePropertyOld(const std::string & name);

  /**
   * Declare the Vector valued property named "name".
   * This must be done _before_ a property of that name is tried
   * to be retrieved using getOld().
   */
  MooseArray<MooseArray<Real> > & declareVectorPropertyOld(const std::string & name);
  
  /**
   * Declare the Tensor valued property named "name".
   * This must be done _before_ a property of that name is tried
   * to be retrieved using getOld().
   */
  MooseArray<RealTensorValue> & declareTensorPropertyOld(const std::string & name);
  
  /**
   * Declare the Tensor valued property named "name".
   * This must be done _before_ a property of that name is tried
   * to be retrieved using getOld().
   */
  MooseArray<ColumnMajorMatrix> & declareColumnMajorMatrixPropertyOld(const std::string & name);
  
  /**
   * Declare the Matrix valued property named "name".
   * This must be done _before_ a property of that name is tried
   * to be retrieved using getOld().
   */
  MooseArray<MooseArray<MooseArray<Real> > > & declareMatrixPropertyOld(const std::string & name);





  /**
   * Declare the Real valued property named "name".
   * This must be done _before_ a property of that name is tried
   * to be retrieved using getOlder().
   */
  MooseArray<Real> & declareRealPropertyOlder(const std::string & name);
  
  /**
   * Declare the Constant Real valued property named "name".
   * This must be done _before_ a property of that name is tried
   * to be retrieved using getOlder().
   */
  Real & declareConstantRealPropertyOlder(const std::string & name);
  
  /**
   * Declare the Gradient valued property named "name".
   * This must be done _before_ a property of that name is tried
   * to be retrieved using getOlder().
   */
  MooseArray<RealGradient> & declareGradientPropertyOlder(const std::string & name);

  /**
   * Declare the RealVectorValue valued property named "name".
   * This must be done _before_ a property of that name is tried
   * to be retrieved using getOlder().
   */
  MooseArray<RealVectorValue> & declareRealVectorValuePropertyOlder(const std::string & name);

  /**
   * Declare the Vector valued property named "name".
   * This must be done _before_ a property of that name is tried
   * to be retrieved using getOlder().
   */
  MooseArray<MooseArray<Real> > & declareVectorPropertyOlder(const std::string & name);
  
  /**
   * Declare the Tensor valued property named "name".
   * This must be done _before_ a property of that name is tried
   * to be retrieved using getOlder().
   */
  MooseArray<RealTensorValue> & declareTensorPropertyOlder(const std::string & name);
  
  /**
   * Declare the Tensor valued property named "name".
   * This must be done _before_ a property of that name is tried
   * to be retrieved using getOlder().
   */
  MooseArray<ColumnMajorMatrix> & declareColumnMajorMatrixPropertyOlder(const std::string & name);
  
  /**
   * Declare the Matrix valued property named "name".
   * This must be done _before_ a property of that name is tried
   * to be retrieved using getOlder().
   */
  MooseArray<MooseArray<MooseArray<Real> > > & declareMatrixPropertyOlder(const std::string & name);


  
  std::map<unsigned int, std::vector<QpData *> > _qp_prev;
  std::map<unsigned int, std::vector<QpData *> > _qp_curr;

  std::vector<std::string> _constant_real_stateful_props;
  std::vector<std::string> _real_stateful_props;
  std::vector<std::string> _gradient_stateful_props;
  std::vector<std::string> _real_vector_value_stateful_props;
  std::vector<std::string> _vector_stateful_props;
  std::vector<std::string> _tensor_stateful_props;
  std::vector<std::string> _column_major_matrix_stateful_props;
  std::vector<std::string> _matrix_stateful_props;


  std::map<std::string, Real >                                       & _constant_real_props;
  std::map<std::string, MooseArray<Real> >                           & _real_props;
  std::map<std::string, MooseArray<RealGradient> >                   & _gradient_props;
  std::map<std::string, MooseArray<RealVectorValue> >                & _real_vector_value_props;
  std::map<std::string, MooseArray<MooseArray<Real> > >              & _vector_props;
  std::map<std::string, MooseArray<RealTensorValue> >                & _tensor_props;
  std::map<std::string, MooseArray<ColumnMajorMatrix> >              & _column_major_matrix_props;
  std::map<std::string, MooseArray<MooseArray<MooseArray<Real> > > > & _matrix_props;

  std::map<std::string, Real >                                       & _constant_real_props_old;
  std::map<std::string, MooseArray<Real> >                           & _real_props_old;
  std::map<std::string, MooseArray<RealGradient> >                   & _gradient_props_old;
  std::map<std::string, MooseArray<RealVectorValue> >                & _real_vector_value_props_old;
  std::map<std::string, MooseArray<MooseArray<Real> > >              & _vector_props_old;
  std::map<std::string, MooseArray<RealTensorValue> >                & _tensor_props_old;
  std::map<std::string, MooseArray<ColumnMajorMatrix> >              & _column_major_matrix_props_old;
  std::map<std::string, MooseArray<MooseArray<MooseArray<Real> > > > & _matrix_props_old;

  std::map<std::string, Real >                                       & _constant_real_props_older;
  std::map<std::string, MooseArray<Real> >                           & _real_props_older;
  std::map<std::string, MooseArray<RealGradient> >                   & _gradient_props_older;
  std::map<std::string, MooseArray<RealVectorValue> >                & _real_vector_value_props_older;
  std::map<std::string, MooseArray<MooseArray<Real> > >              & _vector_props_older;
  std::map<std::string, MooseArray<RealTensorValue> >                & _tensor_props_older;
  std::map<std::string, MooseArray<ColumnMajorMatrix> >              & _column_major_matrix_props_older;
  std::map<std::string, MooseArray<MooseArray<MooseArray<Real> > > > & _matrix_props_older;

  
  // Per elem stateful property data
  std::map<std::string, Real > * _constant_real_props_current_elem;
  std::map<unsigned int, std::map<std::string, MooseArray<Real> > > * _real_props_current_elem;
  std::map<unsigned int, std::map<std::string, MooseArray<RealGradient> > > * _gradient_props_current_elem;
  std::map<unsigned int, std::map<std::string, MooseArray<RealVectorValue> > > * _real_vector_value_props_current_elem;
  std::map<unsigned int, std::map<std::string, MooseArray<MooseArray<Real> > > > * _vector_props_current_elem;
  std::map<unsigned int, std::map<std::string, MooseArray<RealTensorValue> > > * _tensor_props_current_elem;
  std::map<unsigned int, std::map<std::string, MooseArray<ColumnMajorMatrix> > > * _column_major_matrix_props_current_elem;
  std::map<unsigned int, std::map<std::string, MooseArray<MooseArray<MooseArray<Real> > > > > * _matrix_props_current_elem;

  std::map<std::string, Real > * _constant_real_props_old_elem;
  std::map<unsigned int, std::map<std::string, MooseArray<Real> > > * _real_props_old_elem;
  std::map<unsigned int, std::map<std::string, MooseArray<RealGradient> > > * _gradient_props_old_elem;
  std::map<unsigned int, std::map<std::string, MooseArray<RealVectorValue> > > * _real_vector_value_props_old_elem;
  std::map<unsigned int, std::map<std::string, MooseArray<MooseArray<Real> > > > * _vector_props_old_elem;
  std::map<unsigned int, std::map<std::string, MooseArray<RealTensorValue> > > * _tensor_props_old_elem;
  std::map<unsigned int, std::map<std::string, MooseArray<ColumnMajorMatrix> > > * _column_major_matrix_props_old_elem;
  std::map<unsigned int, std::map<std::string, MooseArray<MooseArray<MooseArray<Real> > > > > * _matrix_props_old_elem;

  std::map<std::string, Real > * _constant_real_props_older_elem;
  std::map<unsigned int, std::map<std::string, MooseArray<Real> > > * _real_props_older_elem;
  std::map<unsigned int, std::map<std::string, MooseArray<RealGradient> > > * _gradient_props_older_elem;
  std::map<unsigned int, std::map<std::string, MooseArray<RealVectorValue> > > * _real_vector_value_props_older_elem;
  std::map<unsigned int, std::map<std::string, MooseArray<MooseArray<Real> > > > * _vector_props_older_elem;
  std::map<unsigned int, std::map<std::string, MooseArray<RealTensorValue> > > * _tensor_props_older_elem;
  std::map<unsigned int, std::map<std::string, MooseArray<ColumnMajorMatrix> > > * _column_major_matrix_props_older_elem;
  std::map<unsigned int, std::map<std::string, MooseArray<MooseArray<MooseArray<Real> > > > > * _matrix_props_older_elem;

  //  QGauss * _qrule;
  
//private:
//  Kernel::_qrule;

  /**
   * Whether or not this coupled_as name is associated with an auxiliary variable.
   */
//  bool isAux(std::string name, int i = 0);
};

#endif //MATERIAL_H
