/*
 *      This class contains a set of multi-dimensional interpolation functions for both scattered data and data lying on a cartesian grid
 *
 *      Sources:
 *      - General
 *        * Numerical Recipes in C++ 3rd edition
 *      - MD spline
 *        * Christian Habermann, Fabian Kindermann, "Multidimensional Spline Interpolation: Theory and Applications", Computational Economics, Vol.30-2, pp 153-169 (2007) [http://link.springer.com/article/10.1007%2Fs10614-007-9092-4]
 *      - Inverse distance weighting
 *        * http://en.wikipedia.org/wiki/Inverse_distance_weighting
 *
 */


#ifndef ND_INTERPOLATION_FUNCTIONS_H_
#define ND_INTERPOLATION_FUNCTIONS_H_

#include <vector>
#include <string>

class ND_Interpolation
{
public:
	virtual double interpolateAt(std::vector<double> point_coordinate);
	virtual double getGradientAt(std::vector<double> point_coordinate);
	virtual void   fit(std::vector< std::vector<double> > coordinates, std::vector<double> values);
	std::vector<double> NDinverseFunction(double F_min, double F_max);
	double NDderivative(std::vector<double> x);
	ND_Interpolation();
	~ND_Interpolation();
protected:
	std::string _dataFileName;
	bool        _completedInit;
	double minkowskiDistance(std::vector<double> point1, std::vector<double> point2, double p);
	double vectorNorm(std::vector<double> point, double p);
};

class NDspline: public ND_Interpolation
{
public:
	double interpolateAt(std::vector<double> point_coordinate);
	double getGradientAt(std::vector<double> point_coordinate);
	void   fit(std::vector< std::vector<double> > coordinates, std::vector<double> values);
	NDspline(std::string filename, std::vector<double> alpha, std::vector<double> beta);
	NDspline();
	~NDspline();
private:
    std::vector< std::vector<double> > _discretizations;
	std::vector<double> _values;
	int _dimensions;
	std::vector<double> _splineCoefficients;
	std::vector<double> _hj;
	std::vector<double> _alpha;
	std::vector<double> _beta;

	//void initializeCoefficientsVector();
	void saveCoefficient(double value, std::vector<int> coefficientCoordinate);
	double retrieveCoefficient(std::vector<int> coefficientCoordinate);

	double spline_cartesian_interpolation(std::vector<double> point_coordinate);
	double getPointAtCoordinate(std::vector<int> coordinates);

	int fromNDto1Dconverter(std::vector<int> coordinate);
	std::vector<int> from1DtoNDconverter(int oneDcoordinate, std::vector<int> indexes);

	void calculateCoefficients();
	std::vector<double> fillArrayCoefficient(int nDimensions, std::vector<double> & data, std::vector<int> & loopLocator);

	void from2Dto1Drestructuring(std::vector<std::vector<double> > & twoDdata, std::vector<double> & oneDdata);
	void from1Dto2Drestructuring(std::vector<std::vector<double> > & twoDdata, std::vector<double> & oneDdata, int spacing);

	double phi(double t);
	double u_k(double x, double a, double h, double i);
	void tridag(std::vector<double> & a, std::vector<double> & b, std::vector<double> & c, std::vector<double> & r, std::vector<double> & u);
	std::vector<double> getCoefficients(std::vector<double> & y, double h, double alpha, double beta);
	//void iterationStep(int nDim, std::vector<double> & coefficients, std::vector<double> & data);

	std::vector<double> coefficientRestructuring(std::vector<std::vector<double> > matrix);
	std::vector<std::vector<double> > tensorProductInterpolation(std::vector<std::vector<double> > step1, double h, double alpha, double beta);
	std::vector<std::vector<double> > matrixRestructuring(std::vector<std::vector<double> > step1);
	std::vector<double> getValues(std::vector<int> & loopLocator);
};

class inverseDistanceWeigthing: public ND_Interpolation
{
public:
	double interpolateAt(std::vector<double> point_coordinate);
	double getGradientAt(std::vector<double> point_coordinate);
	void   fit(std::vector< std::vector<double> > coordinates, std::vector<double> values);
	std::vector<double> NDinverseFunction(double F_min, double F_max);
	inverseDistanceWeigthing(std::string filename, double p);
	inverseDistanceWeigthing(double p);
private:
	int _dimensions;
	int _numberOfPoints;
	double _p;
	std::vector<double> _values;
	std::vector< std::vector<double> > _pointCoordinates;
};

class microSphere: public ND_Interpolation
{
public:
	double interpolateAt(std::vector<double> point_coordinate);
	double getGradientAt(std::vector<double> point_coordinate);
	void   fit(std::vector< std::vector<double> > coordinates, std::vector<double> values);
	microSphere(std::string filename, double p, int precision);
	microSphere(double p, int precision);
private:
	int _dimensions;
	int _numberOfPoints;
	double _p;
	std::vector<double> _values;
	std::vector< std::vector<double> > _pointCoordinates;
	int _precision;
	std::vector< std::vector<double> > _unitVector;
	void MSinitialization();
	double cosValueBetweenVectors(std::vector<double> point1, std::vector<double> point2);
};

//class ND_Interpolation_Functions_old{
//public:
//	ND_Interpolation_Functions(int dimensions, std::vector< std::vector<double> > discretizations, std::vector<double> values, ND_interpolation_type type, std::vector<double> alpha, std::vector<double> beta);
//	ND_Interpolation_Functions(std::string filename, ND_interpolation_type type);
//
//    virtual ~ND_Interpolation_Functions();
//
//    double interpolate (std::vector<double> point_coordinate);
//
//    double InverseDistanceWeigthing(std::vector<double> point, std::vector< std::vector<double> > pointCoordinates, std::vector<double> pointValues, double p);
//
//protected:
//    ND_interpolation_type _type;
//    std::vector< std::vector<double> > _discretizations;
//    std::vector<double> _values;
//    int _dimensions;
//    std::vector<double> _splineCoefficients;
//    std::vector<double> _hj;
//    std::vector<double> _alpha;
//    std::vector<double> _beta;
//    int _precision;
//    std::vector< std::vector<double> > _unitVector (_precision, std::vector<double> (_dimensions));
//
//    void initializeCoefficientsVector();
//    void saveCoefficient(double value, std::vector<int> coefficientCoordinate);
//    double retrieveCoefficient(std::vector<int> coefficientCoordinate);
//
//    double spline_cartesian_interpolation(std::vector<double> point_coordinate);
//    double getPointAtCoordinate(std::vector<int> coordinates);
//
//    int fromNDto1Dconverter(std::vector<int> coordinate);
//    std::vector<int> from1DtoNDconverter(int oneDcoordinate);
//
//    void calculateCoefficients();
//    void fillArrayCoefficient(int nDim, std::vector<double> coefficients, std::vector<double> data);
//
//    void from2Dto1Drestructuring(std::vector<std::vector<double> > twoDdata, std::vector<double> oneDdata);
//    void from1Dto2Drestructuring(std::vector<std::vector<double> > twoDdata, std::vector<double> oneDdata, int spacing);
//
//    double phi(double t);
//    double u_k(double x, double a, double h, double i);
//    void tridag(std::vector<double> a, std::vector<double> b, std::vector<double> c, std::vector<double> r, std::vector<double> u);
//    void getCoefficients(std::vector<double> coefficients, std::vector<double> y, double h, double alpha, double beta);
//    void iterationStep(int nDim, std::vector<double> coefficients, std::vector<double> data);
//
//    double minkowskiDistance (std::vector<double> point1, std::vector<double> point2, double p);
//
//    void MSinitialization();
//    double MS();
//};

#endif
