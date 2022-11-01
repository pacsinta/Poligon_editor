# Poligon_editor

## Task description:

1. The vertices of a polygon can be defined in a row by pressing the left mouse button.

2. When the right mouse button is pressed, a new point (not necessarily a pre-existing vertex!) is added on the boundary (the edge of the polygon) closest to the cursor and is inserted between the start and end point of the edge. Until the right mouse button is released, this new vertex will follow the movement of the cursor while the polygon is also deformed.

3. The polygon can be made more curvy by pressing the 's' key, the algorithm for which is as follows. A uniform Catmull-Rom spline of tensor -1 is fitted to the existing vertices, and a spline point corresponding to the arithmetic mean of their parameters is fitted between two existing points.

4. The polygon can be simplified by pressing the 'd' key, which halves the number of vertices in the polygon. The vertices are discarded if discarding them will minimally change the shape of the polygon, i.e. if the distance between the discarded vertex and the new edge is minimal.



## Feladat leírás:

1. A poligon csúcsait sorban a bal egérgomb lenyomásával lehet definiálni.

2. A jobb egérgom lenyomásakor a határon (a poligon élein) a kurzorhoz legközelebb felvesz egy új pontot (nem feltétlenül már létező csúcspont!), és az élszakasz kezdő és végpontja közé beköti. Amíg a jobb egérgombot el nem engedjük ez az új csúcs követi a kurzor mozgását, mialatt a poligon is deformálódik.

3. A poligont az 's' billentyűvel lehet szebben görbülővé tenni, melynek algoritmusa a következő. A meglévő csúcspontokra egy egyenletes, -1-es tenziójú Catmull-Rom splinet illesztünk, és két meglévő pont közé a paramétereik számtani közepének megfelelő spline pontot illesztünk.

4. A poligont az 'd' billentyűvel lehet egyszerűsíteni, melynek hatására a poligon csúcsainak száma megfeleződik. Azokat a csúcsokat dobjuk el, amelyek eldobása minimálisan módosítja a poligon alakját, azaz, amelyenél az eldobott csúcs és a keletkező új él távolsága minimális.
