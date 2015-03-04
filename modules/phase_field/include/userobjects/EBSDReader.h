#ifndef EBSDREADER_H
#define EBSDREADER_H

#include "GeneralUserObject.h"
#include "EBSDAccessFunctors.h"

class EBSDReader;

template<>
InputParameters validParams<EBSDReader>();

/**
 * A GeneralUserObject that reads an EBSD file and stores the centroid
 * data in a data structure which indexes on element centroids.
 */
class EBSDReader : public GeneralUserObject, public EBSDAccessFunctors
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
  const EBSDPointData & getData(const Point & p) const;

  /**
   * Get the requested type of average data for feature number i.
   */
  const EBSDAvgData &  getAvgData(unsigned int i) const;

  /**
   * Get the requested type of average data for a given phase and grain.
   */
  const EBSDAvgData &  getAvgData(unsigned int phase, unsigned int grain) const;

  /**
   * Return the total number of grains
   */
  unsigned int getGrainNum() const;

  /**
   * Return the number of grains in a given phase
   */
  unsigned int getGrainNum(unsigned int phase) const;

protected:
  // MooseMesh Variables
  MooseMesh & _mesh;
  NonlinearSystem & _nl;

  // Variables needed to determine reduced order parameter values
  unsigned int _op_num;
  unsigned int _feature_num;
  Point _bottom_left;
  Point _top_right;
  Point _range;

  // Logically three-dimensional data indexed by geometric points in a 1D vector
  std::vector<EBSDPointData> _data;

  // Averages by feature ID
  std::vector<EBSDAvgData> _avg_data;

  // feature ID for given phases and grains
  std::vector<std::vector<unsigned int> > _feature_id;

  /// Dimension of the problem domain
  unsigned int _mesh_dimension;

  /// The number of values in the x, y and z directions.
  unsigned _nx, _ny, _nz;

  /// The spacing of the values in x, y and z directions.
  Real _dx, _dy, _dz;

  /// Grid origin
  Real _minx, _miny, _minz;

  /// Maximum grid extent
  Real _maxx, _maxy, _maxz;

  // Initial condition values of EBSD variables
  std::vector<Real> _phi1_ic, _PHI_ic, _phi2_ic, _x_ic, _y_ic, _z_ic;
  std::vector<unsigned int> _grn_ic, _phase_ic, _sym_ic;

  // Grain averaged values of EBSD variables
  std::vector<Real> _avg_phi1, _avg_PHI, _avg_phi2, _avg_x, _avg_y, _avg_z;
  std::vector<unsigned int> _avg_phase, _avg_sym;

  /// Computes a global index in the _data array given an input *centroid* point
  unsigned indexFromPoint(const Point & p) const;

  /// Transfer the index into the _avg_data array from given index
  unsigned indexFromIndex(unsigned int var) const;
};

#endif // EBSDREADER_H
