#include "World.h"

World::World() {
	for (unsigned int x = 0; x < 30; x++) {
		for (unsigned int y = 0; y < 10; y++) {
			for (unsigned int z = 0; z < 30; z++) {
				if (y == 0)
					blocks[x][y][z].textureID = 2;
				if (y == 3 && x % 4 != 0 && z % 5 != 0)
					blocks[x][y][z].textureID = 1;
				else if ((x==0||x==9||z==0||z==9) || (x % 9 == 0 && z % 11 == 0))
					blocks[x][y][z].textureID = 1;
				if (y == 9)
					blocks[x][y][z].textureID = 3;
			}
		}
	}

	for (unsigned int i = 0; i < 2160; i++) {
		sines[i] = std::sin(PI*(i / 6.0f)/180);
	}
	colors[0] = sf::Color(0, 0, 0, 255);
	colors[1] = sf::Color(100, 100, 100, 255);
	colors[2] = sf::Color(200, 200, 200, 255);
	colors[3] = sf::Color(200, 0, 0, 255);
	colors[4] = sf::Color(0, 200, 0, 255);
	colors[5] = sf::Color(0, 0, 200, 255);
	textures[0].loadFromFile("Wall.png");
	textures[1].loadFromFile("Floor.png");
	textures[2].loadFromFile("Ceiling.png");
	dynTextures[0].loadFromFile("Dynamic.png");
	UpdateDyn();
}

World::~World() {
}

void World::UpdateScreenVertex(sf::VertexArray* v, int xoff, int yoff)
{
	float hStart = (cam.hrotation + cam.fovV) / 180;
	float hIncreaseBy = cam.fovV / 90 / height;
	float vStart = cam.rotation - cam.fovV/2;
	float vIncreaseBy = cam.fovV / width;
	float rayAngle;
	sf::Vector3f ldir;
	for (int i = xoff; i < width; i+=2) {
		rayAngle = vStart + vIncreaseBy*i;
		if (rayAngle == 0 || rayAngle == 90 || rayAngle == 180 || rayAngle == 270 || rayAngle == 360)
			rayAngle += 0.01f;		//Avoid straight lines
		GetDir(rayAngle, &ldir);
		for (int j = yoff; j < height; j+=2) {
			ldir.y = hStart - j*hIncreaseBy;
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
	cam.rotation = LoopAngle(cam.rotation);
	//std::cout << cam.rotation << "\n";
}

void World::LookUp(float angle)
{
	if (std::abs(cam.hrotation + angle) + cam.fovV < 180)
		cam.hrotation += angle;
}

void World::Move(float forw, float right, float up)
{
	float dirx = Sin(cam.rotation);
	float dirz = Cos(cam.rotation);
	sf::Vector3f ldir = sf::Vector3f(forw * dirx + right * dirz, up, forw * dirz - right * dirx);
	if (blocks[(int)(cam.pos.x+ldir.x)][(int)cam.pos.y][(int)cam.pos.z].textureID != 0)
		ldir.x = 0;
	if (blocks[(int)cam.pos.x][(int)(cam.pos.y+ldir.y)][(int)cam.pos.z].textureID != 0)
		ldir.y = 0;
	if (blocks[(int)cam.pos.x][(int)cam.pos.y][(int)(cam.pos.z+ldir.z)].textureID != 0)
		ldir.z = 0;
	cam.pos += ldir;
	UpdateDyn();
	//std::cout << cam.pos.x << "; " << cam.pos.y << "; " << cam.pos.z << "\n";
}

void World::DynMove(unsigned int index, sf::Vector3f dir)
{
	dyn[index].pos += dir;
	UpdateDyn(index);
}

void World::UpdateDyn(int index)
{
	if (index < 0)
		index = 0;
	sf::Vector3f to = dyn[index].pos - cam.pos;
	dyn[index].distToCamera = to.x * to.x + to.y * to.y + to.z * to.z;
	dyn[index].distToCamera = std::sqrtf(dyn[index].distToCamera);
}

float World::Cos(float angle)
{
	angle += 90;
	angle = LoopAngle(angle);
	return sines[(int)(angle * 6)];
}

float World::Sin(float angle)
{
	angle = LoopAngle(angle);
	return sines[(int)(angle * 6)];
}

float World::LoopAngle(float angle)
{
	return angle > 360 ? angle - 360 : (angle < 0 ? angle + 360 : angle);
}

float World::VAngle(sf::Vector2f a, sf::Vector2f b)
{
	return 180*std::acosf(a.x * b.x + a.y * b.y)/PI;
}

sf::Vector2f World::VNormalize(sf::Vector2f v)
{
	return v / VLength(v);
}

float World::VLength(sf::Vector2f v)
{
	return std::sqrtf(v.x * v.x + v.y * v.y);
}

float World::VAngleXZ(sf::Vector3f a, sf::Vector3f b)
{
	float ang = std::atan2(b.z,b.x) - std::atan2(a.z,a.x);
	return ang >= PI ? ang - 2 * PI : (ang < -PI ? ang + 2 * PI : ang);
}

sf::Vector3f World::VNormalize(sf::Vector3f v)
{
	return v / VLength(v);
}

float World::VLength(sf::Vector3f v)
{
	return std::sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

sf::Color World::Raycast(sf::Vector3f ldir, float rayAngle)
{
	sf::Color c;
	Block* block;
	sf::Image* tex;
	ldir = VNormalize(ldir);
	float dist = 0;
	float tryDist = 0;
	bool dynPassed = false;
	try {
		sf::Vector3f pos = cam.pos;
		sf::Vector3f tryPos = pos;
		for (unsigned int i = 0; i < maxIter; i++) {
			float raySpeed = std::min(std::min(	(ldir.x > 0 ? 1.0f - (pos.x - (int)pos.x) : (pos.x - (int)pos.x)) / std::abs(ldir.x),
												(ldir.y > 0 ? 1.0f - (pos.y - (int)pos.y) : (pos.y - (int)pos.y)) / std::abs(ldir.y)),
												(ldir.z > 0 ? 1.0f - (pos.z - (int)pos.z) : (pos.z - (int)pos.z)) / std::abs(ldir.z));
			raySpeed += 0.002f;
			tryDist = dist + raySpeed;
			tryPos = pos + ldir * raySpeed;

			if (!dynPassed && tryDist >= dyn[0].distToCamera) {			//Dynamic objects
				raySpeed *= (dyn[0].distToCamera - dist) / raySpeed;
				dist = dyn[0].distToCamera;
				pos += ldir * raySpeed;
				sf::Vector3f to = sf::Vector3f(dyn[0].pos.x-pos.x, dyn[0].pos.y-pos.y, dyn[0].pos.z-pos.z);
				float w = to.x * to.x + to.z * to.z;
				if (std::abs(to.y) < dyn[0].size.y && (w < dyn[0].size.x)) {
					float ang = VAngleXZ(ldir, VNormalize(dyn[0].pos - cam.pos))*dist;
					sf::Image* tex = &dynTextures[dyn[0].textureID];
					int x = (0.5f + ang / PI * 0.5f / dyn[0].size.x) * (tex->getSize().x-1);
					int y = (dyn[0].size.y + to.y) / dyn[0].size.y / 2 * (tex->getSize().y-1);
					x = std::max(x, 0); y = std::max(y, 0);
					c = tex->getPixel(x,y);
					if (c.a > 127) {				//Transparency check
						float darken = (2*dist + 2 * Sin(std::abs(rayAngle - cam.rotation))) * dist;
						c.r = std::max(0.0f, c.r - darken); c.g = std::max(0.0f, c.g - darken); c.b = std::max(0.0f, c.b - darken);
						return c;
					}
				}
				dynPassed = true;
			}

			dist = tryDist;
			pos = tryPos;
			block = &blocks[(int)pos.x][(int)pos.y][(int)pos.z];
			if (block->textureID != 0) {
				float darken = (2*dist + 2 * Sin(std::abs(rayAngle - cam.rotation))) * dist;
				if (block->textureID < 0) {
					c = colors[-block->textureID];
				}
				else {		//Get color from texture with coordinates
					tex = &textures[block->textureID - 1];
					if (pos.x - (int)pos.x < 0.01f || pos.x - (int)pos.x > 0.99f) {
						c = tex->getPixel(tex->getSize().x * (pos.z-(int)pos.z), tex->getSize().y * (pos.y-(int)pos.y));
					}
					else if (pos.y - (int)pos.y < 0.01f || pos.y - (int)pos.y > 0.99f) {
						c = tex->getPixel(tex->getSize().x * (pos.x-(int)pos.x), tex->getSize().y * (pos.z-(int)pos.z));
					}
					else if (pos.z - (int)pos.z < 0.01f || pos.z - (int)pos.z > 0.99f) {
						c = tex->getPixel(tex->getSize().x * (pos.x-(int)pos.x), tex->getSize().y * (pos.y-(int)pos.y));
					}
					/*else {
						c = colors[1];
					}*/
				}
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
