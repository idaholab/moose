#!/usr/bin/env python
import statistics
from scipy.stats import norm
import random
import copy


def stat(a):
   n = len(a)
   a_bar = statistics.mean(a)
   return a_bar
   #return sum([(a[i] - a_bar)**2 for i in range(n)])/n

def jackknife(a, func):
   out = []
   n = len(a)
   for i in range(n):
      a_jk = [a[j] for j in range(n) if j != i]
      out.append(func(a_jk))
   return out


if __name__ == '__main__':
   A = [48,36,20,29,42,42,20,42,22,41,45,14,6,0,33,28,34,4,32,24,47,41,24,26,30,41]

   N = 1000
   theta = stat(A)
   print('theta =', theta)

   theta_b = list()
   for i in range(N):
      Ab = [A[random.randint(0, len(A)-1)] for j in range(len(A))]
      theta_b.append(stat(Ab))

   cnt = 0
   for b in theta_b:
      if b < theta:
         cnt += 1

   z0 = norm.ppf(cnt/N)

   n = len(A)
   jk = jackknife(A, stat)
   theta_dot = sum(jk)/n

   top = sum([(theta_dot - jk[i])**3 for i in range(n)])
   bot = sum([(theta_dot - jk[i])**2 for i in range(n)])
   acc = top / (6*bot**(3/2))
   print('count =', cnt, 'bias =', z0, 'acc =', acc)
