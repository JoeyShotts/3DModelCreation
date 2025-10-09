https://en.wikipedia.org/wiki/STL_(file_format)

**Binary .stl:** 
Designed for large stl files. It has a 80 byte header (typically initialized to 0) a 4 byte unsigned integer (little-endianess) indicating the number of triangles in the mesh, and then 12 floats per triangle. 1 point indicating a vector normal to the trangle (this can be (0,0,0) and the software will calculate with RHR) and 3 XYZ points.

https://en.wikipedia.org/wiki/Endianness
Big Endianness: Most Significant Byte First: 00000101 00000001 = 0x0501 = 1281
Little Endianness: Least Significant Byte First: 00000101 00000001 = 0x0105 = 261

**The Normal Vector:**
This is crucial as it defines the outside and inside of the 3D model. The normal vector should always point out. It should be a unit vector, where the magnitude of the XYZ components should equal one. Additionally the unit vector created by the order of the three points (using the Right Hand Rule) should match the Normal Vector.
Calculating the Normal Vector:
https://web.ma.utexas.edu/users/m408m/Display12-5-4.shtml

**ASCII .stl Files:**
Designed as a more human readable format of .stl files.


