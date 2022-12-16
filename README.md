# concave-hull

A function written to convert a QVector of QPointFs into a concave QPolygonF.

At the current time the program is buggy but will generate some nice concave polygons.

![image](https://user-images.githubusercontent.com/72110751/208163656-46eeca13-7054-4274-b101-2fee91222dd8.png)

There is a tendency to exclude points rather then create a tangled polygon.

# TODO

The right_most_point functions angle manipulation is a little wonky. When this is fixed changes will likely have to be made to the concave_hull function too.
