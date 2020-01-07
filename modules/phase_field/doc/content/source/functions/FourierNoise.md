# FourierNoise

!syntax description /Functions/FourierNoise

This object generates a Fourier series with random coefficients $s_{\vec k}$,
$c_{\vec k}$, and an amplitude factor $\sigma$ (`scale`). The series can be cut
off at a given length scale $\lambda$ (`lambda`) resulting in a low pass
filtered white noise. For efficiency lambda should be chosen larger than the
smallest mesh element size.

\begin{equation}
\sigma \sum_{\vc k, |\vec k| < \frac 1\lambda} s_{\vec k}\sin \vec r\cdot \vec k + c_{\vec k}\cos \vec r\cdot \vec k
\end{equation}

Alternatively the series can be constructed with an entirely random set of
series terms by specifying the number of terms `num_terms` resulting in a
non-periodic noise.

!syntax parameters /Functions/FourierNoise

!syntax inputs /Functions/FourierNoise

!syntax children /Functions/FourierNoise

!bibtex bibliography
