# Computing equilibrium temperature or activity

This page follows Chapter 11 of [!cite](bethke_2007).

The `geochemistry` module can be used to provide equilibrium temperature or activity.  For instance, using the standard GWB database and anydrite instead of Ca$^{2+}$ as a component (using a [swap](swap.md)), the reaction
\begin{equation}
\mathrm{gypsum}\rightleftharpoons \mathrm{anhydrite} + 2\mathrm{H}_{2}\mathrm{O} \ ,
\end{equation}
has the equilibrium equation
\begin{equation}
\log_{10}K = 2\log_{10}a_{\mathrm{H}_{2}\mathrm{O}} \ .
\end{equation}
Then:

- If water activity is 1, the equilibrium temperature (where $K=1$) is found to be 43.7$^{\circ}$C.  MOOSE produces the result ????
- If water activity is 0.7, the equilibrium temperature (where $K=1$) is found to be 11.8$^{\circ}$C.  MOOSE produces the result ????
- If the temperature is 25$^{\circ}$C, the equilibrium is attained when water activity is 0.815.  MOOSE produces the result ????

!bibtex bibliography