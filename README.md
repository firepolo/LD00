# LD00
Little 3D project for familiarization with new OpenGL, OpenAL and map generation.

![alt tag](https://raw.githubusercontent.com/firepolo/LD00/master/resources/doc/preview.png)

## Requirement
- OpenGL 3.3 (minimum)
- OpenAL
- SDL2
- GLEW

## Keyboard inputs (QWERTZ)
- Close window : ESCAPE
- Forward : W or UP
- Backward : S or DOWN
- Turn left : LEFT
- Turn right : RIGHT
- Straffe left : S
- Straffe right : D
- Hit : SPACE

## Map generation
I use a very simple algorithm for create the random map.

On grid, I move a point to a random direction

![alt tag](https://raw.githubusercontent.com/firepolo/LD00/master/resources/doc/move.png)

Above, when the point move back on this own way (yellow point in exemple), the point move to random direction.
When the point move on a new point (blank point in exemple), the point move to random direction and increment map size.
Stop algorithm when the map size equals to function argument.

```c++
std::vector<Point> points;
const Point dirs[] = { { -1, 0 }, { 0, -1 }, { 1, 0 }, { 0, 1 } };

Point p = { 0 };

while (points.size() < size)
{
	Point d = dirs[Random::GetNumber<GLuint>(0, 4)];
	Point n = { p.x + d.x, p.y + d.y };
	if (std::find(points.begin(), points.end(), n) == points.end())
	{
		if (n.x < l) l = n.x;
		else if (n.x > r) r = n.x;
		if (n.y < t) t = n.y;
		else if (n.y > b) b = n.y;
		points.push_back(n);
	}
	*((int *)&p) = *((int *)&n);
}
```

## References
- https://www.khronos.org/files/opengl45-quick-reference-card.pdf
- https://openal.org/