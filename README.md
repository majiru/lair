# LAIR 1 COMS327-SPRING-2019

## NAME
Lair

## SYNOPSIS
Lair takes no arguments

## DESCRIPTION
Lair is a rogue lite dungeon crawler built using plan9's libdraw.

## BUILDING

* plan9: `mk`
* unix: `make`

## BUGS
Resize is undefined behavior.

## NOTES
To grading TA -- This code includes vendored libs that are already assembled inside of lib. 
The code itself relies on the plan9's libdraw graphical system. This lib has been ported under 
a larger collection of software called plan9port.
I compiled this library header stubs on pyrite and they should work on most linux systems.
I would reccomend the use of the fedora VM, being the closest to pyrite itself.
I included these libs because the whole project itself can be quite large, and wanted to 
provide something quick and easy to grade.
