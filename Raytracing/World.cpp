#include "World.h"

World::World() {
	for (unsigned int x = 0; x < 100; x++) {
		for (unsigned int y = 0; y < 3; y++) {
			for (unsigned int z = 0; z < 100; z++) {
				if (y == 0)
					blocks[x][y][z].textureID = -3 - (x + z) % 3;
				if ((x==0||x==99||z==0||z==99) || (y == 1 && x % 3 == 0 && z % 5 == 0))
					blocks[x][y][z].textureID = -1 - (x + z) % 2;
				if (y == 2)
					blocks[x][y][z].textureID = -3 - (x + z) % 3;
			}
		}
	}

	for (unsigned int i = 0; i < 2160; i++) {
		sines[i] = std::sin(PI*(i / 6.0f)/180);
	}
	for (unsigned int i = 0; i < colorMults; i++) {
		colorMult[i] = sf::Color(255 * i / (float)(colorMults-1), 255 * i / (float)(colorMults - 1), 255 * i / (float)(colorMults - 1), 0);
	}
	colors[0] = sf::Color(0, 0, 0, 255);
	colors[1] = sf::Color(100, 100, 100, 255);
	colors[2] = sf::Color(200, 200, 200, 255);
	colors[3] = sf::Color(200, 0, 0, 255);
	colors[4] = sf::Color(0, 200, 0, 255);
	colors[5] = sf::Color(0, 0, 200, 255);
}

World::~World() {
}

void World::UpdateScreenVertex(sf::VertexArray* v, int xoff, int yoff)
{
	float hIncreaseBy = 1.0f / height;
	float rayAngle;
	sf::Vector3f ldir;
	for (int i = xoff; i < width; i+=2) {
		rayAngle = cam.rotation + ((i - width / 2) / (float)width) * cam.fovH;
		GetDir(rayAngle, &ldir);
		for (int j = yoff; j < height; j+=2) {
			//dir.y = Sin(((height/2 - j) / (float)height) * cam.fovV);
			ldir.y = 0.5f - j*hIncreaseBy;
			(*v)[i + j * width].color = Raycast(ldir,rayAngle);
		}
	}
}

void World::GetDir(float angle, sf::Vector3f* dir)
{
	dir->z = Cos(angle);
	dir->x = Sin(angle);
}

void World::Turn(float angle)
{
	cam.rotation += angle;
	if (cam.rotation > 360)
		cam.rotation -= 360;
	if (cam.rotation < 0)
		cam.rotation += 360;
	//std::cout << cam.rotation << "\n";
}

void World::Move(float forw, float right)
{
	float dirx = Sin(cam.rotation);
	float dirz = Cos(cam.rotation);
	cam.pos += sf::Vector3f(forw*dirx+right*dirz,0,forw*dirz-right*dirx);
	//std::cout << cam.pos.x << "; " << cam.pos.y << "; " << cam.pos.z << "\n";
}

float World::Cos(float angle)
{
	angle += 90;
	if (angle < 0)
		angle += 360;
	if (angle >= 360)
		angle -= 360;
	return sines[(int)(angle * 6)];
}

float World::Sin(float angle)
{
	if (angle < 0)
		angle += 360;
	if (angle >= 360)
		angle -= 360;
	return sines[(int)(angle * 6)];
}

sf::Color World::Raycast(sf::Vector3f ldir, float rayAngle)
{
	sf::Color c;
	try {
		sf::Vector3f pos = cam.pos;
		for (unsigned int i = 0; i < maxIter; i++) {
			float raySpeed = std::min(std::min((ldir.x > 0 ? 1.0f - (pos.x - (int)pos.x) : (pos.x - (int)pos.x))/std::abs(ldir.x), 
				(ldir.y > 0 ? 1.0f - (pos.y - (int)pos.y) : (pos.y - (int)pos.y)) / std::abs(ldir.y)),
				(ldir.z > 0 ? 1.0f - (pos.z - (int)pos.z) : (pos.z - (int)pos.z)) / std::abs(ldir.z));
			raySpeed = std::max(0.001f, raySpeed);
			pos += ldir * raySpeed;
			if (blocks[(int)pos.x][(int)pos.y][(int)pos.z].textureID < 0) {
				float distS = (pos.x - cam.pos.x) * (pos.x - cam.pos.x) + (pos.y - cam.pos.y) * (pos.y - cam.pos.y) + (pos.z - cam.pos.z) * (pos.z - cam.pos.z);
				float darken = (2 + 4 * Sin(std::abs(rayAngle - cam.rotation))) * distS;
				c = colors[-blocks[(int)pos.x][(int)pos.y][(int)pos.z].textureID];
				c.r = std::max(0.0f, c.r - darken); c.g = std::max(0.0f, c.g - darken); c.b = std::max(0.0f, c.b - darken);
				return c;
			}
		}
		return colors[0];
	}
	catch (std::exception e) {
		std::cout << e.what() << "\n";
		return colors[0];
	}
}
