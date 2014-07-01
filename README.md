# Star=CD Addons (Profe Macros and Macro Dependencies)

These Star-CD(profe module) macros are intended to be called from Star-CD v2.6 environment.

The following is a brief description for each macro:


## Divc

Macro name: divc2d.MAC
Dependency: divc.exe

Splits a layer of structured 2d mesh into two so that each layer is also structured. 

The code is only intended to work with the profe macro divc2d.MAC.


## BendOverCylinder

Macro name: divc2d.MAC
Dependency: divc.exe

Transforms the selected nodes so that they bend over a pipe with outside radius R. 

This operation preserves the volume.


## Jacobian

Macro name: Jacobian.MAC
Dependency: Jacobian.py

Improves mesh quality of 3d hexahedral mesh by reducing negative volume cells.

The script moves the node that constitutes (with 3 neighboring nodes) the lowest jacobian in a cell along the outward direction.

The operation is not guaranteed and, especially when the general mesh quality is bad, might make the jacobinas even worse.

