#ifndef QPDATA_H
#define QPDATA_H

/**
 * Holds a data structure used to compute material properties at a Quadrature point
 */
class QpData 
{
//private:

public:
//  QpData();

  virtual ~QpData(){}

  virtual QpData& operator=(QpData &) {}

  virtual QpData& operator=(const QpData &) {}
  
//  virtual QpData * getData() const = 0; 

//  virtual QpData * createData() {}
};

#endif //QPDATA_H
