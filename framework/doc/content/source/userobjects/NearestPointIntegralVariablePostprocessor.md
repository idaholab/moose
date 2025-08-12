# NearestPointIntegralVariablePostprocessor

The domain is virtually divided into a number of subdomains according to the
nearest points provided by users. And then the variable integral is taken over
each individual subdomain separately.

!alert warning
Note that if [!param](/VectorPostprocessors/NearestPointIntegralVariablePostprocessor/positions_object) is used to specify the nearest points,
only the vector of initial [Positions](syntax/Positions/index.md) are used at this time.
Updates to the 'positions' vector are not supported.

!syntax description /VectorPostprocessors/NearestPointIntegralVariablePostprocessor

!syntax parameters /VectorPostprocessors/NearestPointIntegralVariablePostprocessor

!syntax inputs /VectorPostprocessors/NearestPointIntegralVariablePostprocessor

!syntax children /VectorPostprocessors/NearestPointIntegralVariablePostprocessor

!bibtex bibliography
