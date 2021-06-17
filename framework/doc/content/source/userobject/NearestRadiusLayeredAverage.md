# NearestRadiusLayeredAverage

The domain is virtually divided into a number of rings according to the
points provided by users. For each point provided, the radius, or
distance to the origin in the plane perpendicular to the direction provided
is computed. Any point of the domain with a radius closest to that radius
contributes to that ring. It is thus recommended to input points along a straight
 line, with each point being located in the middle of the ring.  And then the
variable average is taken over each individual ring separately with the ability
 to decompose each subdomain in layers along a particular direction. If the
number of layers is larger than one, it is recommended that the interface between
 two layers align with the mesh.

!syntax description /UserObjects/NearestRadiusLayeredAverage

!syntax parameters /UserObjects/NearestRadiusLayeredAverage

!syntax inputs /UserObjects/NearestRadiusLayeredAverage

!syntax children /UserObjects/NearestRadiusLayeredAverage

!bibtex bibliography
