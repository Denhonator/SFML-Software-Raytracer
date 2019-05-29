#include "World.h"

World::World() {
	for (unsigned int i = 0; i < 10; i++) {
		lights[i].pos.x = 6 + (i % 2) * 11;
		lights[i].pos.z = 3 + 6 * (i%5);
		lights[i].pos.y = 1.9f + 5 * (i > 4 ? 1 : 0);
	}
	for (unsigned int x = 0; x < 30; x++) {
		for (unsigned int y = 0; y < 10; y++) {
			for (unsigned int z = 0; z < 30; z++) {
				if (y == 0)
					blocks[x][y][z].textureID = 2;
				if ((x == 0 || x == 29 || z == 0 || z == 29) || (x % 9 == 0 && z % 5 == 0))
					blocks[x][y][z].textureID = 1;
				else if (y == 4 && x % 3 != 0 && z % 4 != 0)
					blocks[x][y][z].textureID = 4;
				if (y == 9)
					blocks[x][y][z].textureID = 3;
			}
		}
	}
	for (unsigned int x = 0; x < 30; x++) {
		for (unsigned int y = 0; y < 10; y++) {
			for (unsigned int z = 0; z < 30; z++) {
				for (unsigned int i = 0; i < 6; i++) {		//Lights
					blocks[x][y][z].l[i] = -1;
					float dist = 1000;
					for (unsigned int j = 0; j < 10; j++) {
						if (i == 0 && lights[j].pos.x > x + 1 ||
							i == 1 && lights[j].pos.x < x ||
							i == 2 && lights[j].pos.y > y + 1 ||
							i == 3 && lights[j].pos.y < y ||
							i == 4 && lights[j].pos.z > z + 1 ||
							i == 5 && lights[j].pos.z < z) {
							sf::Vector3f testPos = sf::Vector3f(x + (i == 0 ? 1 : i == 1 ? 0 : 0.5f), y + (i == 2 ? 1 : i == 3 ? 0 : 0.5f), z + (i == 4 ? 1 : i == 5 ? 0 : 0.5f));
							float tryDist = VLength(lights[j].pos - testPos);
							if (tryDist < dist && tryDist < 8) {
								for (unsigned int k = 0; k < 10; k++) {
									for (unsigned int p = 0; p < 10; p++) {
										if (i < 2) {
											testPos.y = (int)testPos.y + k * 0.1f;
											testPos.z = (int)testPos.z + p * 0.1f;
										}
										else if (i < 4) {
											testPos.x = (int)testPos.x + k * 0.1f;
											testPos.z = (int)testPos.z + p * 0.1f;
										}
										else {
											testPos.x = (int)testPos.x + k * 0.1f;
											testPos.y = (int)testPos.y + p * 0.1f;
										}
										rays[0].pos = testPos;
										rays[0].dir = lights[j].pos - testPos;
										rays[0].maxDist = tryDist;
										rays[0].dir = VNormalize(rays[0].dir);
										if (LRaycast(&rays[0])) {
											blocks[x][y][z].l[i] = j;
											dist = tryDist;
											k = 99;
											blocks[x][y][z].lit += std::max(lights[j].intensity / dist / dist - dist * 0.01f, 0.0f);
											break;
										}
									}
								}
							}
						}
					}
				}
			}
		}
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
	textures[3].loadFromFile("Block.png");
	dynTextures[0].loadFromFile("Dynamic.png");

	for (unsigned int i = 0; i < 5; i++) {
		dyn[i].textureID = 0;
		dyn[i].pos.x += i*0.5f;
	}

	cam.fovH *= PI / 180.0f;
	cam.fovV *= PI / 180.0f;
}

World::~World() {
}

void World::UpdateScreenVertex(sf::VertexArray* v, short num, short cycle)
{
	short xoff = (num & 1) * 2 + (cycle & 2 ? 1 : 0);
	short yoff = (num & 2) + (cycle & 1);
	float vStart = cam.fovV / 2;
	float vIncreaseBy = cam.fovV / height;
	float vOff = std::sin(cam.hrotation);
	float hStart = cam.rotation - cam.fovH/2;
	float hIncreaseBy = cam.fovH / width;
	float hrayAngle;
	float vrayAngle;
	Ray* r = &rays[num];
	for (int i = xoff; i < width; i+=4) {
		hrayAngle = (hStart + hIncreaseBy*i);
		if (hrayAngle == 0 || hrayAngle == PIH || hrayAngle == PI || hrayAngle == PI+PIH || hrayAngle == PI2)
			hrayAngle += 0.001f;		//Avoid straight lines
		r->angle = hrayAngle;
		r->dir.x = std::sin(hrayAngle); r->dir.z = std::cos(hrayAngle);
		float vDeDistort = std::cos(cam.rotation - hrayAngle);		//Cos fixes distortion on edges

		for (int j = yoff; j < height; j+=4) {
			vrayAngle = (vStart - j * vIncreaseBy);
			r->dir.y = (vOff + std::sin(vrayAngle))*vDeDistort;
			r->yscale = std::cos(cam.hrotation + vrayAngle);
			Raycast(r);
			(*v)[i + width * j].color = r->c;
		}
	}
}

void World::UpdateWorld()
{
	for (unsigned int i = 0; i < 10; i++) {
		DynMove(i, sf::Vector3f((int)clock.getElapsedTime().asSeconds() % 2 - 0.5f, 0, (int)(clock.getElapsedTime().asSeconds() + 0.5f) % 2 - 0.5f) * 0.03f);
		/*float sine = std::abs(std::sin(clock.getElapsedTime().asSeconds()/4));
		if(i%2)
			lights[i].c.g = sine*255;
		else
			lights[i].c.b = sine*255;*/
	}
	if (cam.speed.x != 0 || cam.speed.z != 0 || cam.speed.y != 0 || cam.airtime>0) {
		Move(cam.speed.x, cam.speed.z, cam.speed.y);
	}
	UpdateDyn();
}

void World::Move(float forw, float right)
{
	if(forw<2)
		cam.speed.x = forw;
	if(right<2)
		cam.speed.z = right;
}

void World::Turn(float angle)
{
	cam.rotation += angle*cam.speedM;
	cam.rotation = LoopAngle(cam.rotation);
	//std::cout << cam.rotation << "\n";
}

void World::LookUp(float angle)
{
	angle *= cam.speedM;
	if (std::abs(cam.hrotation + angle) + cam.fovV/2 < PIH)
		cam.hrotation += angle;
	//std::cout << cam.hrotation / PI * 180 << "\n";
}

void World::Move(float forw, float right, float up)
{
	cam.speed.y -= 0.002f;
	float dirx = std::sin(cam.rotation);
	float dirz = std::cos(cam.rotation);
	float xlimits[2] = { -0.1f,0.1f }; float ylimits[2] = { -0.8f,0.1f }; float zlimits[2] = { -0.1f,0.1f };
	sf::Vector3f ldir = sf::Vector3f(forw * dirx + right * dirz, up, forw * dirz - right * dirx);
	ldir.x *= cam.speedM; ldir.z *= cam.speedM;
	sf::Vector3i testPos;
	for (unsigned int x = 0; x < 2; x++) {
		for (unsigned int y = 0; y < 2; y++) {
			for (unsigned int z = 0; z < 2; z++) {
				testPos.x = (int)(cam.pos.x + ldir.x + xlimits[x]); testPos.y = (int)(cam.pos.y + ylimits[y]); testPos.z = (int)(cam.pos.z + zlimits[z]);
				if (blocks[testPos.x][testPos.y][testPos.z].textureID != 0)
					ldir.x = 0;
				testPos.x = (int)(cam.pos.x + xlimits[x]); testPos.y = (int)(cam.pos.y + ldir.y + ylimits[y]);
				if (blocks[testPos.x][testPos.y][testPos.z].textureID != 0) {
					ldir.y = 0;
					cam.speed.y = 0;
				}
				testPos.y = (int)(cam.pos.y + ylimits[y]); testPos.z = (int)(cam.pos.z + ldir.z + zlimits[z]);
				if (blocks[testPos.x][testPos.y][testPos.z].textureID != 0)
					ldir.z = 0;
			}
		}
	}
	if (ldir.x || ldir.y || ldir.z)
		cam.airtime = 0.2f;
	else if (cam.airtime > 0)
		cam.airtime -= 0.016f;		//TODO
	cam.pos += ldir;
	//std::cout << cam.pos.x << "; " << cam.pos.y << "; " << cam.pos.z << "\n";
}

void World::Jump(float speed)
{
	cam.speed.y = speed;
}

void World::DynMove(unsigned int index, sf::Vector3f dir)
{
	dyn[index].pos += dir;
	dyn[index].lit += (blocks[(int)dyn[index].pos.x][(int)dyn[index].pos.y][(int)dyn[index].pos.z].lit - dyn[index].lit)*0.1f;
}

void World::UpdateDyn()
{
	for (unsigned int i = 0; i < 10; i++) {
		if (dyn[i].textureID<0)
			continue;
		sf::Vector3f to = dyn[i].pos - cam.pos;
		dyn[i].distToCamera = to.x * to.x + to.z * to.z;
		dyn[i].distToCamera = std::sqrtf(dyn[i].distToCamera);

		if (i>0 && dyn[i].distToCamera < dyn[i-1].distToCamera) {
			Dynamic temp = dyn[i - 1];
			dyn[i - 1] = dyn[i];
			dyn[i] = temp;
		}
	}
}

inline float World::LoopAngle(float angle)
{
	return angle > PI2 ? angle - PI2 : (angle < 0 ? angle + PI2 : angle);
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
	float dist = 0;
	sf::Vector3f pos = cam.pos;
	sf::Vector3i posi(pos.x, pos.y, pos.z);
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
	unsigned short DI = 0;

	for (unsigned int i = 0; i < maxIter; i++) {
		float xray = (dirxadd + dirxsign * (pos.x - posi.x)) / dirxlen;
		float yray = (diryadd + dirysign * (pos.y - posi.y)) / dirylen;
		float zray = (dirzadd + dirzsign * (pos.z - posi.z)) / dirzlen;
		float raySpeed = std::min({ xray, yray,	zray });			//Rayspeed matches what is needed to reach next block

		raySpeed += 0.002f;											//Add a little on top so it doesn't fall short
		float tryDist = dist + raySpeed;
		tryPos += dir * raySpeed;

		
		while (DI<10 && tryDist >= dyn[DI].distToCamera) {			//Dynamic objects
			raySpeed = dyn[DI].distToCamera - dist;					//Reduce rayspeed to hit object
			dist = dyn[DI].distToCamera;
			pos += dir * raySpeed;									//Set new position
			sf::Vector3f to = dyn[DI].pos-pos;						//Vector from ray pos to object pos (middle, y ignored)
			float w = to.x * to.x + to.z * to.z;
			float sizey = dyn[DI].size.y * r->yscale;
			if (std::abs(to.y) < sizey && (w < dyn[DI].size.x)) {
				float ang = VAngleXZ(dir, VNormalizeXZ(dyn[DI].pos - cam.pos)) * dist;
				sf::Image* tex = &dynTextures[dyn[DI].textureID];
				sf::Vector2u tsize = tex->getSize();
				int x = (0.5f + ang / PI * 0.5f / dyn[DI].size.x) * (tsize.x - 1);
				int y = (sizey + to.y) / sizey / 2 * (tsize.y - 1);
				x = std::max(x, 0); y = std::max(y, 0);
				sf::Color c = tex->getPixel(x, y);
				if (c.a > 127) {				//Transparency check
					//float lit = 1.5f / (dist + std::abs(cam.pos.y - pos.y) + std::sin(std::abs(r->angle - cam.rotation))) - dist * 0.01f;
					//lit = std::min(std::max(lit, 0.0f),1.0f);
					float lit = dyn[DI].lit;
					c.r = c.r * lit; c.g = c.g * lit; c.b = c.b * lit;
					r->c = c;
					return;
				}
			}
			DI++;
		}

		dist = tryDist;
		pos = tryPos;
		posi.x = pos.x; posi.y = pos.y; posi.z = pos.z;
		//if (pos.x < 0 || pos.y < 0 || pos.z < 0 || pos.x >= 30 || pos.y >= 10 || pos.z >= 30)	//Check out of bounds. Shouldn't be necessary if area is covered
		//	break;
		Block* block = &blocks[posi.x][posi.y][posi.z];
		if (block->textureID != 0) {
			sf::Color c;
			int lindex = 0;
			xray -= dir.x * 0.001f;
			float hdif = pos.y - cam.pos.y;
			if(hdif*(pos.y-posi.y-0.5f)<0)
				yray -= std::abs(hdif)*0.1f;
			zray -= dir.z * 0.001f;
			if (yray < xray && yray < zray)
				lindex = 2 + ((1 - dirysign) >> 1);
			else if (xray < yray && xray < zray)
				lindex = (1 - dirxsign) >> 1;
			else if (zray < yray && zray < xray)
				lindex = 4 + ((1 - dirzsign) >> 1);

			if (block->textureID < 0) {
				c = colors[-block->textureID];
			}
			else {		//Get color from texture with coordinates
				sf::Image* tex = &textures[block->textureID - 1];
				sf::Vector2u tsize = tex->getSize();
				if (lindex<2) {
					c = tex->getPixel(tsize.x * (pos.z-posi.z), tsize.y * (pos.y-posi.y));
				}
				else if (lindex<4) {
					c = tex->getPixel(tsize.x * (pos.x-posi.x), tsize.y * (pos.z-posi.z));
				}
				else {
					c = tex->getPixel(tsize.x * (pos.x-posi.x), tsize.y * (pos.y-posi.y));
				}
			}
			float lit = std::max(0.1f / dist - dist * 0.0001f, 0.02f);
			/*if(dist<2)
				lit = 0.2f / (dist + std::abs(cam.pos.y - pos.y) + std::sin(std::abs(r->angle - cam.rotation)));*/
			lindex = block->l[lindex];
			if (lindex >= 0) {
				sf::Vector3f newDir = lights[lindex].pos - pos;
				dist = VLength(newDir);
				r->maxDist = dist;
				newDir = VNormalize(newDir);
				r->dir = newDir;
				r->pos = pos;
				if (LRaycast(r)) {
					lit += std::max(lights[lindex].intensity / dist / dist - dist*0.01f,0.0f);
					c *= lights[lindex].c;
				}
				r->dir = dir;
			}
			lit = std::max(lit, 0.0f);
			c.r = std::min(c.r * lit, 255.0f); c.g = std::min(c.g * lit, 255.0f); c.b = std::min(c.b * lit, 255.0f);
			r->c = c;
			return;
		}
	}
	r->c = sf::Color::Black;
}

bool World::LRaycast(Ray* r)
{
	float dist = 0;
	sf::Vector3f pos = r->pos;
	sf::Vector3i posi(pos.x, pos.y, pos.z);
	sf::Vector3f dir = r->dir;
	float maxDist = r->maxDist;
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
		float raySpeed = std::min({ (dirxadd + dirxsign * (pos.x - posi.x)) / dirxlen,		//Rayspeed matches what is needed to reach next block
									(diryadd + dirysign * (pos.y - posi.y)) / dirylen,
									(dirzadd + dirzsign * (pos.z - posi.z)) / dirzlen });
		raySpeed += 0.002f;											//Add a little on top so it doesn't fall short
		dist += raySpeed;
		if (dist >= maxDist)
			return true;
		pos += dir*raySpeed;
		posi.x = pos.x; posi.y = pos.y; posi.z = pos.z;
		
		//if (pos.x < 0 || pos.y < 0 || pos.z < 0 || pos.x >= 30 || pos.y >= 10 || pos.z >= 30)	//Check out of bounds. Shouldn't be necessary if area is covered
		//	break;
		Block* block = &blocks[posi.x][posi.y][posi.z];
		if (block->textureID != 0) {
			return false;
		}
	}
	return false;
}
