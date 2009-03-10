#ifndef QPDATA_H
#define QPDATA_H

/**
 * Holds a data structure used to compute material properties at a Quadrature point
 */
struct QpData 
{
  virtual ~QpData(){}

  inline virtual QpData& operator=(const QpData &) { return *this; }
};

#endif //QPDATA_H
