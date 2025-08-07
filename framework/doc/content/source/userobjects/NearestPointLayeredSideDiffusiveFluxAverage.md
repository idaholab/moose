# NearestPointLayeredSideDiffusiveFluxAverage

The domain is virtually divided into a number of layered subdivision according
to the nearest points and the layering direction provided by users. And then
the layered side average diffusive flux is computed for the sides on each
individual subdivision separately.

!alert warning
Note that if [!param](/UserObjects/NearestPointLayeredSideDiffusiveFluxAverage/positions_object) is used to specify the nearest points,
only the vector of initial [Positions](syntax/Positions/index.md) are used at this time.
Updates to the 'positions' vector are not supported.

!syntax description /UserObjects/NearestPointLayeredSideDiffusiveFluxAverage

!syntax parameters /UserObjects/NearestPointLayeredSideDiffusiveFluxAverage

!syntax inputs /UserObjects/NearestPointLayeredSideDiffusiveFluxAverage

!syntax children /UserObjects/NearestPointLayeredSideDiffusiveFluxAverage

!bibtex bibliography
