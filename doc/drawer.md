## Drawer
Have you ever wanted to visualize tests for geometry problems? Jngen gives you a convenient way to do so. It gives an instrument for drawing
basic geometric primitives (points, circles, segments and polygons) in SVG format.

<img src=pics/img1.png align=left width=28% />
<img src=pics/img2.png align=left width=36% />
<img src=pics/img3.png align=left width=28% />

<br />

Here is a usage example.

```cpp
// Create an instance of a Drawer class
Drawer d;

// Use Point or Pointf from jngen or your own point class.
// In the latter case it must have two fields named x and y.
// Both integers and reals are supported.
Point p1(3, 14);
Point p2(15, 92);

d.point(p1);
// Second argument is radius
d.circle(p1, 5);
d.segment(p1, p2);
// d.polygon takes vector or initializer list of points as its argument
d.polygon(vector<Point>{p1, p2, Point{1, 2}, Point{5, 6}});

// You can also use pairs:
d.point(pair<double, double>(0.5, 1.1));
d.circle(pair<int, int>(5, 6), 10);
d.segment(make_pair(1, 2), make_pair(3, 4));
d.polygon(vector<pair<int, int>>{ {0, 0}, {0, 10}, {10, 0} });

// Or even specify coordinates by hand for point, circle and segment:
d.point(1, 2);
d.circle(5, 10, 3.3);
// Here the order is x1, y1, x2, y2
d.segment(0, 0, 10, 10);

// Style of figures can be altered. Any style change only applies
// to figures which were drawn after.

// You can change the color of your figures...
d.setColor("green");
// and deal with stroke and fill separately:
d.setStroke("red");
d.setFill("blue");
// Both stroke and fill can be set to none passing an empty string:
d.setFill("");
// You can use any color which is supported by HTML/SVG. If the color
// has adequate name it is likely on the list.

// It is possible to set line width (default is 1):
d.setWidth(2.5);
// And opacity (ranging from 0 to 1, 0 is invisible, 1 is solid):
d.setOpacity(0.5);

// By default Jngen draws a cool grid with coordinates. I find it
// very handy, however, if you don't like it it is easy do disable:
d.enableGrid(false);

// Finally, you should save your piece of art to the SVG file:
d.dumpSvg("name.svg");
```
