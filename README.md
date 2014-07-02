# Star-CD Addons (Profe Macros and Macro Dependencies)

These Star-CD(profe module) macros are intended to be called from Star-CD v2.6 environment.

The following is a brief description for each macro:


## Divc

Macro name: divc2d.MAC

Dependency: divc.exe

Splits a layer of structured 2d mesh into two so that each layer is also structured. 

The code is only intended to work with the profe macro divc2d.MAC.


## BendOverCylinder

Macro name: bend_over_pipe.MAC

Transforms the selected nodes so that they bend over a pipe with outside radius R. 

This operation preserves the volume.


## Jacobian

Macro name: Jacobian.MAC

Dependency: Jacobian.py

Improves mesh quality of 3d hexahedral mesh by reducing negative volume cells.

The script moves the node that constitutes (with 3 neighboring nodes) the lowest Jacobian value in a cell along the outward direction.

The operation is not guaranteed and, especially when the general mesh quality is bad, might make the Jacobian values even worse.


## Extrude

Macro name: extrude.MAC

Extrudes surface shells along vset (one layer of solid per vertex.)

vset is sorted along the local active coordinate system beforehand.

Note: Shells to be extruded are in ctype 5000, ctype 5001 is the extruded solid cell type and ctype 5002 is a temporary shell type that gets deleted on completion.



## Extrude3

Macro name: extrude3.MAC

Extrudes surface shells along 3 vsets, retrievable by rs1, ts2, and rs3.

First vset defines extrusion direction, second vset defines out-of-plane skew angle and the third defined in-plane skew.

The extruded solid is deformed uniformly so that each consecutive three nodes form a face for the extruded solid. 

vset's are sorted along the local active coordinate system beforehand. ctypes 5000, 5001, and 5002 are overwritten.

Note: Shells to be extruded are in ctype 80. ctype 81 is the extruded solid cell type. Coordinate systems 50 and 51 are overwritten.


## Fill

Macro name: Fill.MAC

Uniformly Rearranges nodes in vset.

Rearrangement is done according to the spline the nodes define. For less than 100 nodes, the vfill operation defined by profe might yield better results.

