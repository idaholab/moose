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
   * Returns the Euler angle "phi1" at Point p in O(1) access time.
   */
  Real phi1(const Point& p) const;

  /**
   * Returns the Euler angle "PHI" at Point p in O(1) access time.
   */
  Real phi(const Point& p) const;

  /**
   * Returns the Euler angle "phi2" at Point p in O(1) access time.
   */
  Real phi2(const Point& p) const;

  /**
   * Returns the centroid x-coordinate from the EBSD data at Point p in O(1) access time.
   */
  Real x_centroid(const Point& p) const;

  /**
   * Returns the centroid y-coordinate from the EBSD data at Point p in O(1) access time.
   */
  Real y_centroid(const Point& p) const;

  /**
   * Returns the centroid z-coordinate from the EBSD data at Point p in O(1) access time.
   */
  Real z_centroid(const Point& p) const;

  /**
   * Returns the grain number at Point p in O(1) access time.
   */
  unsigned grain(const Point& p) const;

  /**
   * Returns the phase number at Point p in O(1) access time.
   */
  unsigned phase(const Point& p) const;

  /**
   * Returns the symmetry class at Point p in O(1) access time.
   */
  unsigned symmetry_class(const Point& p) const;

protected:
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
