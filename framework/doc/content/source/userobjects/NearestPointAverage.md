# NearestPointAverage

!syntax description /UserObjects/NearestPointAverage

The domain is virtually divided into a number of subdomains according to
the nearest points provided by the users. For each provided point, a virtual
subdomain is created around that point and consists of all regions of space
that are closer to that point than any of the other provided points. Then,
the variable average is computed over each individual subdomain separately.

!syntax parameters /UserObjects/NearestPointAverage

!syntax inputs /UserObjects/NearestPointAverage

!syntax children /UserObjects/NearestPointAverage
