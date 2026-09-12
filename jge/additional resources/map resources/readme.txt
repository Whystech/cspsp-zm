Unofficial CSPSP Editor v1.60 by coolguy5678
This editor creates levels for CSPSP version 1.60 and up, although
levels that don't use certain advanced features are compatible with
earlier versions.

----------

Changelog

v1.60
- Added custom buyzone editing.
- Removed the default textures, to encourage people to use
  their own textures.

v1.56
- Added an option to remove the grid snap
- Added support for grenades
- Minor: Invalid maps can be saved, so they can be fixed later, but it gives a
  warning
- Minor: lowered the size of the vertex squares slightly to make complex shapes
  easier to edit

v1.50
- Added support for player only collision polygons, which people can't move through,
  but bullets can (like the old wallhack, but less hacky).
- Minor: Removed the requirement that all maps must have weapons, since online
  games don't need guns for the AI players.
- Minor: Changed the map loading code to be more forgiving.

v1.21
- Added a splash screen
- Minor: changed how spawn points are handled, meaning human-edited maps
  have more compatibility (I doubt anyone hand-edits maps anyway).
- Ability to remove textures added
- Texture replacer added

v1.2
- Added support for CSPSP v1.2's new map format. To load an old map
  that uses the old format, selected "Import v1.01 map" from the
  File menu.
- Added new weapons from CSPSP
- Tile Mode: Scrollbar added to texture window
- A few other minor changes

v1.1
- Tile Mode: You can now drag the mouse to fill lots of tiles at once
- Tile Mode: Support for maps with more than 16 textures added
- Screenshot taker added (takes a picture of the entire map
  without gridlines, polygons and waypoints)
- Confirmation to save before exiting and creating new maps added

v1.0
- Initial release

----------

To start, simply double-click ucspspe.jar .
Make sure you have a Java Runtime Environment* installed on your computer.
The latest JRE can be downloaded from:
http://java.sun.com/javase/downloads/index.jsp
(choose the option "Java Runtime Environment (JRE) 6")

For a full manual and a tutorial, you would visit http://coolguy5678.biaklan.com/,
but that's broken. Try using the Wayback Machine (Google it if you haven't heard of it).

----------

Included is a simple example map, called "example". There is also a file
called "default.png" which includes the three textures that are included
in any new maps created. You may change this file, but it's recommended
that you keep the dimensions the same.

----------

NOTE TO JAVA DEVELOPERS:
The source is included in the jar file. Feel free to look at, modify and
release modified versions of the program, just make sure you give me
credit.

----------

*To test if this is installed, enter "java -version" into a command prompt.
Version 1.6 is required.