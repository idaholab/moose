# NearestPointLayeredIntegral

The domain is virtually divided into a number of subdomains according to the
nearest points provided by users. And then the variable integral
is taken over each individual subdomain separately with the ability to decompose
each subdomain in layers along a particular direction. If the number of layers is
larger than one, it is recommended that the interface between two layers align
with the mesh.

!alert warning
Note that if [!param](/UserObjects/NearestPointLayeredIntegral/positions_object) is used to specify the nearest points,
only the vector of initial [Positions](syntax/Positions/index.md) are used at this time.
Updates to the 'positions' vector are not supported.

!syntax description /UserObjects/NearestPointLayeredIntegral

!syntax parameters /UserObjects/NearestPointLayeredIntegral

!syntax inputs /UserObjects/NearestPointLayeredIntegral

!syntax children /UserObjects/NearestPointLayeredIntegral

!bibtex bibliography
