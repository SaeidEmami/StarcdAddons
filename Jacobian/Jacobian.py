###################################################################################################
# Available for use under the [MIT License](http://en.wikipedia.org/wiki/MIT_License)             #
# Writer: Saeid Emami                                                                             #
###################################################################################################

# Improves mesh quality of 3d hexahedral mesh by reducing negative volume cells.
#
# The script moves the node that constitutes (with 3 neighboring nodes) the lowest jacobian in a cell along the outward direction.
#
# The operation is not guaranteed and, especially when the general mesh quality is bad, might make the jacobinas even worse.


from math import sqrt, atan, pi
import time
import os

JRATIOMAX = 30 # Ratio of node displacememnt to the height of the contructed tetrahedral around a node.
ITERATIONS = 300 # Number of iterations the script would try.

def dot(a , b):
  (a1, a2, a3) = a
  (b1, b2, b3) = b
  return a1 * b1 + a2 * b2 + a3 * b3

def cross(a, b):
  (a1, a2, a3) = a
  (b1, b2, b3) = b
  c1 = a2 * b3 - a3 * b2
  c2 = a3 * b1 - a1 * b3
  c3 = a1 * b2 - a2 * b1
  return (c1, c2, c3)

def length(a):
  (a1, a2, a3) = a
  return sqrt(a1 ** 2 + a2 ** 2 + a3 ** 2)

def normal(V0, V1, V2, V3):
  '''normal to (V1, V2, V3) plane towards V0  where 123 point away to out side (not V0) using rigth-hand convention.'''
  (x0, y0, z0) = V0 
  (x1, y1, z1) = V1 
  (x2, y2, z2) = V2 
  (x3, y3, z3) = V3 
  (n1, n2, n3) = cross((x3 - x1, y3 - y1, z3 - z1), (x2 - x1, y2 - y1, z2 - z1))
  l = length((n1, n2, n3))
  return (n1 /l, n2 /l, n3 / l)

def area2(V1, V2, V3):
  '''(Twice the) Area of <V1, V2, V3> tirangle'''
  (x1, y1, z1) = V1 
  (x2, y2, z2) = V2 
  (x3, y3, z3) = V3 
  (a1, a2, a3) = cross((x3 - x1, y3 - y1, z3 - z1), (x2 - x1, y2 - y1, z2 - z1))
  return length((a1, a2, a3))

def distance(V0, V1, V2, V3):
  '''Distance of V0 from <V1, V2, V3> plane'''
  n = normal(V0, V1, V2, V3)
  return abs(dot(n, (V0[0] - V1[0] , V0[1] - V1[1], V0[2] - V1[2])))

def smallestedge(V1, V2, V3, V4, V5, V6, V7, V8):
  (x1, y1, z1) = V1 
  (x2, y2, z2) = V2 
  (x3, y3, z3) = V3 
  (x4, y4, z4) = V4 
  (x5, y5, z5) = V5 
  (x6, y6, z6) = V6 
  (x7, y7, z7) = V7 
  (x8, y8, z8) = V8 
  l12 = length((x1 - x2, y1 - y2, z1 - z2))
  l14 = length((x1 - x4, y1 - y4, z1 - z4))
  l15 = length((x1 - x5, y1 - y5, z1 - z5))
  l23 = length((x2 - x3, y2 - y3, z2 - z3))
  l26 = length((x2 - x6, y2 - y6, z2 - z6))
  l34 = length((x3 - x4, y3 - y4, z3 - z4))
  l37 = length((x3 - x7, y3 - y7, z3 - z7))
  l48 = length((x4 - x8, y4 - y8, z4 - z8))
  l56 = length((x5 - x6, y5 - y6, z5 - z6))
  l58 = length((x5 - x8, y5 - y8, z5 - z8))
  l67 = length((x6 - x7, y6 - y7, z6 - z7))
  l78 = length((x7 - x8, y7 - y8, z7 - z8))
  lmin = min(l12, l14, l15, l23, l26, l34, l37, l48, l56, l58, l67, l78)
  return lmin

def jacob(V0, V1, V2, V3):
  a11 = V1[0] - V0[0]
  a12 = V2[0] - V0[0]
  a13 = V3[0] - V0[0]
  a21 = V1[1] - V0[1]
  a22 = V2[1] - V0[1]
  a23 = V3[1] - V0[1]
  a31 = V1[2] - V0[2]
  a32 = V2[2] - V0[2]
  a33 = V3[2] - V0[2]
  return a11 * a22 * a33 + a12 * a23 * a31 + a13 * a21 * a32 - a11 * a23 * a32 - a12 * a21 * a33 - a13 * a22 * a31

def Jacob(V1, V2, V3, V4, V5, V6, V7, V8):
  J1 = jacob(V1, V2, V4, V5)
  J2 = jacob(V2, V3, V1, V6)
  J3 = jacob(V3, V2, V7, V4)
  J4 = jacob(V4, V1, V3, V8)
  J5 = jacob(V5, V6, V1, V8)
  J6 = jacob(V6, V5, V7, V2)
  J7 = jacob(V7, V6, V8, V3)
  J8 = jacob(V8, V7, V5, V4)
  Jmax = max(J1, J2, J3, J4, J5, J6, J7, J8)
  Jmin = min(J1, J2, J3, J4, J5, J6, J7, J8)  
  if Jmin <= 0:
    return -1
  return Jmax / Jmin


def modify(c):
  def expandtriangle(v0, v1, v2, v3, v4, v5, v6, v7, direction, delta):
    (V0, V1, V2, V3) = (vset[v0], vset[v1], vset[v2], vset[v3])
    if v0 not in vfixed:
      V0[0] += direction[0] * delta
      V0[1] += direction[1] * delta
      V0[2] += direction[2] * delta
    for vi in (v1, v2, v3):
      if vi not in vfixed:
        Vi = vset[vi]
        Vi[0] -= direction[0] * delta
        Vi[1] -= direction[1] * delta
        Vi[2] -= direction[2] * delta
    #for vi in (v4, v5, v6, v7):
    #  if vi not in vfixed:
    #    Vi = vset[vi]
    #    Vi[0] -= direction[0] * 0.5 * delta
    #    Vi[1] -= direction[1] * 0.5 * delta
    #    Vi[2] -= direction[2] * 0.5 * delta

  def modifytriangle(v0, v1, v2, v3, v4, v5, v6, v7, d):
    (V0, V1, V2, V3) = (vset[v0], vset[v1], vset[v2], vset[v3])
    n = normal(V0, V1, V2, V3)
    expandtriangle(v0, v1, v2, v3, v4, v5, v6, v7, n, d)    
   
  (v1, v2, v3, v4, v5, v6, v7, v8) = cset[c]
  (V1, V2, V3, V4, V5, V6, V7, V8) = (vset[v1], vset[v2], vset[v3], vset[v4], vset[v5], vset[v6], vset[v7], vset[v8])
  J1 = jacob(V1, V2, V4, V5)
  J2 = jacob(V2, V3, V1, V6)
  J3 = jacob(V3, V2, V7, V4)
  J4 = jacob(V4, V1, V3, V8)
  J5 = jacob(V5, V6, V1, V8)
  J6 = jacob(V6, V5, V7, V2)
  J7 = jacob(V7, V6, V8, V3)
  J8 = jacob(V8, V7, V5, V4)
  Jmax = max(J1, J2, J3, J4, J5, J6, J7, J8)
  r = 0.1
  if J1 < min(J2, J3, J4, J5, J6, J7, J8):
    d = Jmax * r / area2(V2, V4, V5) / JRATIOMAX
    modifytriangle(v1, v2, v4, v5, v3, v6, v7, v8, d)
  elif J2 < min(J3, J4, J5, J6, J7, J8):
    d = Jmax * r / area2(V3, V1, V6) / JRATIOMAX
    modifytriangle(v2, v3, v1, v6, v4, v5, v7, v8, d)
  elif J3 < min(J4, J5, J6, J7, J8):
    d = Jmax * r / area2(V2, V7, V4) / JRATIOMAX
    modifytriangle(v3, v2, v7, v4, v1, v5, v6, v8, d)
  elif J4 < min(J1, J2, J3, J5):
    d = Jmax * r / area2(V1, V3, V8) / JRATIOMAX
    modifytriangle(v4, v1, v3, v8, v2, v5, v6, v7, d)
  elif J5 < min(J6, J7, J8):
    d = Jmax * r / area2(V6, V1, V8) / JRATIOMAX
    modifytriangle(v5, v6, v1, v8, v2, v3, v4, v7, d)
  elif J6 < min(J7, J8):
    d = Jmax * r / area2(V5, V7, V2) / JRATIOMAX
    modifytriangle(v6, v5, v7, v2, v1, v3, v4, v8, d)
  elif J7 < J8:
    d = Jmax * r / area2(V6, V8, V3) / JRATIOMAX
    modifytriangle(v7, v6, v8, v3, v1, v2, v4, v5, d)
  else:
    d = Jmax * r / area2(V7, V5, V4) / JRATIOMAX
    modifytriangle(v8, v7, v5, v4, v1, v2, v3, v6, d)
  
now = time.ctime(time.time())
print now

print 'Importing data...'
cwd = os.getcwd() + os.sep

vset = {}
vsetFile = open(cwd + 'jac.vrt', 'r')
for v in vsetFile:
  n = int(v[0:7])
  x = float(v[8:23])
  y = float(v[23:38])
  z = float(v[38:53])
  vset[n] = [x, y, z]
vsetFile.close()

vfixed = []
vfixedFile = open(cwd + 'jac_fixed.vrt', 'r')
for v in vfixedFile:
  n = int(v[0:7])
  vfixed.append(n)
vfixedFile.close()

cset = {}
csetFile = open(cwd + 'jac.crt', 'r')
for c in csetFile:
  n  = int(c.split()[0])
  n1 = int(c.split()[1])
  n2 = int(c.split()[2])
  n3 = int(c.split()[3])
  n4 = int(c.split()[4])
  n5 = int(c.split()[5])
  n6 = int(c.split()[6])
  n7 = int(c.split()[7])
  n8 = int(c.split()[8])
  cset[n] = (n1, n2, n3, n4, n5, n6, n7, n8)
csetFile.close()

print 'Checking the Jacobians...'
badc0 = []
for c in cset:
  (v1, v2, v3, v4, v5, v6, v7, v8) = cset[c]
  (V1, V2, V3, V4, V5, V6, V7, V8) = (vset[v1], vset[v2], vset[v3], vset[v4], vset[v5], vset[v6], vset[v7], vset[v8])
  J = Jacob(V1, V2, V3, V4, V5, V6, V7, V8) 
  if J < 0 or J > JRATIOMAX:
    badc0.append(c)
print 'Number of bad cells: ' + str(len(badc0))

print 'Modifying bad cells...'
for i in range(ITERATIONS):
  for c in cset:
    (v1, v2, v3, v4, v5, v6, v7, v8) = cset[c]
    (V1, V2, V3, V4, V5, V6, V7, V8) = (vset[v1], vset[v2], vset[v3], vset[v4], vset[v5], vset[v6], vset[v7], vset[v8])
    J = Jacob(V1, V2, V3, V4, V5, V6, V7, V8) 
    if J < 0 or J > JRATIOMAX * 0.9:
      modify(c)

print 'Checking the Jacobians...'
badc = []
for c in cset:
  (v1, v2, v3, v4, v5, v6, v7, v8) = cset[c]
  (V1, V2, V3, V4, V5, V6, V7, V8) = (vset[v1], vset[v2], vset[v3], vset[v4], vset[v5], vset[v6], vset[v7], vset[v8])
  J = Jacob(V1, V2, V3, V4, V5, V6, V7, V8) 
  if J < 0 or J > JRATIOMAX:
    badc.append(c)
print 'Number of bad cells: ' + str(len(badc))


print 'Exporting data...'
voFile = open(cwd + 'vo.vrt', 'w')
for v in vset:
  if v not in vfixed:
    V = vset[v]
    l = '%(number)7d %(x) 15.8E%(y) 15.8E%(z) 15.8E\n' % {"number" : v, "x": V[0], "y" : V[1], "z": V[2]}
    voFile.write(l)
voFile.close()

