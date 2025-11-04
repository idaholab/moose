# NearestPointLayeredSideIntegral

The domain is virtually divided into a number of subdomains according to the
nearest points provided by users. And then the layered side integral
is computed for the sides on each individual subdomain separately.

!alert warning
Note that if [!param](/UserObjects/NearestPointLayeredSideIntegral/positions_object) is used to specify the nearest points,
only the vector of initial [Positions](syntax/Positions/index.md) are used at this time.
Updates to the 'positions' vector are not supported.

!syntax description /UserObjects/NearestPointLayeredSideIntegral

!syntax parameters /UserObjects/NearestPointLayeredSideIntegral

!syntax inputs /UserObjects/NearestPointLayeredSideIntegral

!syntax children /UserObjects/NearestPointLayeredSideIntegral

!bibtex bibliography
