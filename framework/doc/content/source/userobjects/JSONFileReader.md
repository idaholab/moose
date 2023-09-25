# JSONFileReader

!syntax description /UserObjects/JSONFileReader

This user object loads JSON file into a `nlohmann::json` object. The data can then be accessed programmatically
using the right key (if directly at the root level of the JSON) or group of keys through the APIs provided by the `JSONFileReader`.

!alert note
There is currently no search feature implemented. The exact path through the JSON tree to the data of interest
must be used.

!syntax parameters /UserObjects/JSONFileReader

!syntax inputs /UserObjects/JSONFileReader

!syntax children /UserObjects/JSONFileReader

!bibtex bibliography
