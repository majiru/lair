# LAIR 1 COMS327-SPRING-2019

## NAME
Lair

## SYNOPSIS
Lair takes no arguments

## DESCRIPTION
Lair is a rogue lite dungeon crawler built using plan9's libdraw.

## CONTROLS
* Backspace, Delete, and Q: Quit the game
* Kesc: Exit the current menu
* i: Display inventory
* w: Equip an item from inventory
* t: Remove an equiped item
* d: Drop an item to the floor
* x: Destroy item from inventory
* e: Display currently equiped items
* I: Show item description
* L: Enter inspection mode, movement moves cursor around, press L again over a monster to view desc
* < and >: Move down and up a floor respectivly
* ,: Pick up an item on the floor
* Space, 5 or . : Wait in place
* Movement can be done with arrow keys, hjkl, and numpad

## DEBUG CONTROLS
* D: Draw djikstra path values
* H: Draw hardness values of cells
* T: Draw tunneling djikstra path values
* m: Display monster menu for current floor
* P: Turn on teleport mode, move around and press P again to land
* f: Toggle fog of war
* s: Force a redraw

## BUILDING
* plan9: `mk`

* unix:
	* Without Plan9Port (Assuming vendored): `make`
	* With Plan9Port Installed: `9 make lair`
