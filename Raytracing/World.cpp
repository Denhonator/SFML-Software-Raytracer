#include "World.h"

World::World() {
	for (unsigned int x = 0; x < 30; x++) {
		for (unsigned int y = 0; y < 10; y++) {
			for (unsigned int z = 0; z < 30; z++) {
				if (y == 0)
					blocks[x][y][z].textureID = 2;
				if (y == 3 && x % 4 != 0 && z % 5 != 0)
					blocks[x][y][z].textureID = 1;
				else if ((x==0||x==29||z==0||z==29) || (x % 9 == 0 && z % 11 == 0))
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

void World::UpdateScreenVertex(sf::VertexArray* v, short num, short cycle)
{
	short xoff = (num & 1) * 2 + (cycle & 2 ? 1 : 0);
	short yoff = (num & 2) + (cycle & 1);
	float vStart = cam.hrotation + cam.fovV/2;
	float vIncreaseBy = cam.fovV / height;
	float hStart = cam.rotation - cam.fovH/2;
	float hIncreaseBy = cam.fovH / width;
	float hrayAngle;
	float vrayAngle;
	Ray* r = &rays[num];
	for (int i = xoff; i < width; i+=4) {
		hrayAngle = hStart + hIncreaseBy*i;
		if (hrayAngle == 0 || hrayAngle == 90 || hrayAngle == 180 || hrayAngle == 270 || hrayAngle == 360)
			hrayAngle += 0.01f;		//Avoid straight lines
		r->angle = hrayAngle;
		r->dir.x = Sin(hrayAngle); r->dir.z = Cos(hrayAngle);
		for (int j = yoff; j < height; j+=4) {
			vrayAngle = (vStart - j * vIncreaseBy) * Cos(cam.rotation-hrayAngle);		//Cos fixes distortion on edges
			if (std::abs(vrayAngle) < 0.005f)
				vrayAngle += 0.01f;
			r->dir.y = Sin(vrayAngle);
			Raycast(r);
			(*v)[i + width * j].color = r->c;
		}
	}
}

void World::UpdateWorld()
{
	DynMove(0, sf::Vector3f((int)clock.getElapsedTime().asSeconds() % 2 - 0.5f, 0, (int)(clock.getElapsedTime().asSeconds() + 0.5f) % 2 - 0.5f) * 0.1f);
	Move(0, 0, cam.ySpeed);
	cam.ySpeed -= 0.002f;
	UpdateDyn();
}

void World::Turn(float angle)
{
	cam.rotation += angle;
	cam.rotation = LoopAngle(cam.rotation);
	//std::cout << cam.rotation << "\n";
}

void World::LookUp(float angle)
{
	if (std::abs(cam.hrotation + angle) + cam.fovV < 90)
		cam.hrotation += angle;
}

void World::Move(float forw, float right, float up)
{
	float dirx = Sin(cam.rotation);
	float dirz = Cos(cam.rotation);
	float xlimits[2] = { -0.1f,0.1f }; float ylimits[2] = { -0.5f,0.1f }; float zlimits[2] = { -0.1f,0.1f };
	sf::Vector3f ldir = sf::Vector3f(forw * dirx + right * dirz, up, forw * dirz - right * dirx);
	for (unsigned int x = 0; x < 2; x++) {
		for (unsigned int y = 0; y < 2; y++) {
			for (unsigned int z = 0; z < 2; z++) {
				if (blocks[(int)(cam.pos.x + ldir.x + xlimits[x])][(int)(cam.pos.y + ylimits[y])][(int)(cam.pos.z+zlimits[z])].textureID != 0)
					ldir.x = 0;
				if (blocks[(int)(cam.pos.x + xlimits[x])][(int)(cam.pos.y + ldir.y + ylimits[y])][(int)(cam.pos.z + zlimits[z])].textureID != 0) {
					ldir.y = 0;
					cam.ySpeed = 0;
				}
				if (blocks[(int)(cam.pos.x + xlimits[x])][(int)(cam.pos.y + ylimits[y])][(int)(cam.pos.z + ldir.z + zlimits[z])].textureID != 0)
					ldir.z = 0;
			}
		}
	}
	cam.pos += ldir;
	//std::cout << cam.pos.x << "; " << cam.pos.y << "; " << cam.pos.z << "\n";
}

void World::Jump(float speed)
{
	cam.ySpeed = speed;
}

void World::DynMove(unsigned int index, sf::Vector3f dir)
{
	dyn[index].pos += dir;
}

void World::UpdateDyn(int index)
{
	if (index < 0)
		index = 0;
	sf::Vector3f to = dyn[index].pos - cam.pos;
	dyn[index].distToCamera = to.x * to.x + to.z * to.z;
	dyn[index].distToCamera = std::sqrtf(dyn[index].distToCamera);
}

float World::Cos(float angle)
{
	angle += 90;
	angle = LoopAngle(angle);
	return sines[(int)(angle * 6)];
}

inline float World::Sin(float angle)
{
	return sines[(int)(LoopAngle(angle) * 6)];
}

inline float World::LoopAngle(float angle)
{
	return angle > 360 ? angle - 360 : (angle < 0 ? angle + 360 : angle);
}

inline float World::VAngle(sf::Vector2f a, sf::Vector2f b)
{
	return 180*std::acosf(a.x * b.x + a.y * b.y)/PI;
}

inline sf::Vector2f World::VNormalize(sf::Vector2f v)
{
	return v / VLength(v);
}

inline float World::VLength(sf::Vector2f v)
{
	return std::sqrtf(v.x * v.x + v.y * v.y);
}

inline float World::VAngleXZ(sf::Vector3f a, sf::Vector3f b)
{
	float ang = std::atan2(b.z,b.x) - std::atan2(a.z,a.x);
	return ang >= PI ? ang - 2 * PI : (ang < -PI ? ang + 2 * PI : ang);
}

inline sf::Vector3f World::VNormalize(sf::Vector3f v)
{
	return v / VLength(v);
}

inline sf::Vector3f World::VNormalizeXZ(sf::Vector3f v)
{
	return v / VLengthXZ(v);
}

inline float World::VLength(sf::Vector3f v)
{
	return std::sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

inline float World::VLengthXZ(sf::Vector3f v)
{
	return std::sqrtf(v.x*v.x+v.z*v.z);
}

void World::Raycast(Ray* r)
{
	bool dynPassed = false;

	float dist = 0;
	sf::Vector3f pos = cam.pos;
	sf::Vector3f tryPos = pos;
	sf::Vector3f dir = r->dir;
	float dirxadd = dir.x > 0 ? 1.0f : 0;
	float diryadd = dir.y > 0 ? 1.0f : 0;
	float dirzadd = dir.z > 0 ? 1.0f : 0;
	short dirxsign = dir.x > 0 ? -1 : 1;
	short dirysign = dir.y > 0 ? -1 : 1;
	short dirzsign = dir.z > 0 ? -1 : 1;
	float dirxlen = std::abs(dir.x);
	float dirylen = std::abs(dir.y);
	float dirzlen = std::abs(dir.z);

	for (unsigned int i = 0; i < maxIter; i++) {
		float raySpeed = std::min({ (dirxadd + dirxsign * (pos.x - (int)pos.x)) / dirxlen,		//Rayspeed matches what is needed to reach next block
									(diryadd + dirysign * (pos.y - (int)pos.y)) / dirylen,
									(dirzadd + dirzsign * (pos.z - (int)pos.z)) / dirzlen });
		raySpeed += 0.002f;											//Add a little on top so it doesn't fall short
		float tryDist = dist + raySpeed;
		tryPos += dir * raySpeed;

		if (!dynPassed && tryDist >= dyn[0].distToCamera) {			//Dynamic objects
			raySpeed = (dyn[0].distToCamera - dist);				//Reduce rayspeed to hit object
			dist = dyn[0].distToCamera;
			pos += dir * raySpeed;									//Set new position
			sf::Vector3f to = dyn[0].pos-pos;						//Vector from ray pos to object pos (middle, y ignored)
			float w = to.x * to.x + to.z * to.z;
			if (std::abs(to.y) < dyn[0].size.y && (w < dyn[0].size.x)) {
				float ang = VAngleXZ(dir, VNormalizeXZ(dyn[0].pos - cam.pos))*dist;
				sf::Image* tex = &dynTextures[dyn[0].textureID];
				sf::Vector2u tsize = tex->getSize();
				int x = (0.5f + ang / PI * 0.5f / dyn[0].size.x) * (tsize.x-1);
				int y = (dyn[0].size.y + to.y) / dyn[0].size.y / 2 * (tsize.y-1);
				x = std::max(x, 0); y = std::max(y, 0);
				sf::Color c = tex->getPixel(x,y);
				if (c.a > 127) {				//Transparency check
					float darken = (1.5f * dist + 2 * Sin(std::abs(r->angle - cam.rotation))) * dist;
					c.r = std::max(0.0f, c.r - darken); c.g = std::max(0.0f, c.g - darken); c.b = std::max(0.0f, c.b - darken);
					r->c = c;
					return;
				}
			}
			dynPassed = true;
		}

		dist = tryDist;
		pos = tryPos;
		if (pos.x < 0 || pos.y < 0 || pos.z < 0 || pos.x >= 30 || pos.y >= 10 || pos.z >= 30)	//Check out of bounds. Shouldn't be necessary if area is covered
			break;
		Block* block = &blocks[(int)pos.x][(int)pos.y][(int)pos.z];
		if (block->textureID != 0) {
			sf::Color c;
			if (block->textureID < 0) {
				c = colors[-block->textureID];
			}
			else {		//Get color from texture with coordinates
				sf::Image* tex = &textures[block->textureID - 1];
				sf::Vector2u tsize = tex->getSize();
				if (pos.x - (int)pos.x < 0.01f || pos.x - (int)pos.x > 0.99f) {
					c = tex->getPixel(tsize.x * (pos.z-(int)pos.z), tsize.y * (pos.y-(int)pos.y));
				}
				else if (pos.y - (int)pos.y < 0.01f || pos.y - (int)pos.y > 0.99f) {
					c = tex->getPixel(tsize.x * (pos.x-(int)pos.x), tsize.y * (pos.z-(int)pos.z));
				}
				else if (pos.z - (int)pos.z < 0.01f || pos.z - (int)pos.z > 0.99f) {
					c = tex->getPixel(tsize.x * (pos.x-(int)pos.x), tsize.y * (pos.y-(int)pos.y));
				}
				/*else {
					c = colors[1];
				}*/
			}
			float darken = (1.5f * dist + 2 * Sin(std::abs(r->angle - cam.rotation))) * dist;
			c.r = std::max(0.0f, c.r - darken); c.g = std::max(0.0f, c.g - darken); c.b = std::max(0.0f, c.b - darken);
			r->c = c;
			return;
		}
	}
	r->c = sf::Color::Black;
}
