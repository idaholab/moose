#ifndef EBSDREADER_H
#define EBSDREADER_H

#include "GeneralUserObject.h"

class EBSDReader;

template<>
InputParameters validParams<EBSDReader>();

/**
 * A GeneralUserObject that reads an EBSD file and stores the centroid
 * data in a data structure which indexes on element centroids.
 */
class EBSDReader : public GeneralUserObject
{
public:
  EBSDReader(const std::string & name, InputParameters params);
  virtual ~EBSDReader();

  /**
   * Called before execute() is ever called so that data can be cleared.
   */
  virtual void initialize() {}

  /**
   * Called when this object needs to compute something.
   */
  virtual void execute() {}

  /**
   * Called _after_ execute(), could be used to do MPI communictaion.
   */
  virtual void finalize() {}

  /**
   * Get the requested type of data at the point p.
   */
  Real get_data(const Point & p, MooseEnum data_type) const;

  /**
   * Returns a MooseEnum object associated with the C++ DataType enum
   * defined in this class.  This should be maintained in the same order
   * as the C++ DataType enum.
   */
  MooseEnum getDataType (const std::string & type) const
    {
      return MooseEnum("PHI1, PHI, PHI2, X, Y, Z, GRAIN, PHASE, SYMMETRY_CLASS", type);
    }

protected:

  /**
   * An enum, which is used in conjunction with a MooseEnum, for
   * determining which type of data to return.
   */
  enum DataType
  {
    PHI1=0,
    PHI,
    PHI2,
    X,
    Y,
    Z,
    GRAIN,
    PHASE,
    SYMMETRY_CLASS
  };

  /// Name of the file containing the EBSD data
  std::string _filename;

  /// Logically three-dimensional data indexed by geometric points in
  /// a 1D vector.
  std::vector<std::vector<Real> > _data;

  /// The number of values in the x, y and z directions.
  unsigned _nx, _ny, _nz;

  /// The spacing of the values in x, y and z directions.
  Real _dx, _dy, _dz;

  /// Computes a global index in the _data array given an input
  /// *centroid* point.
  unsigned index_from_point(const Point& p) const;
};

#endif /* EBSDREADER_H */
