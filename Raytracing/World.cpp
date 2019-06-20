#include "World.h"

World::World() {
	srand(time(NULL));
	alights.reserve(sizeof(Light*) * 100);
	for (int x = 0; x < 100; x++) {
		for (int y = 0; y < 10; y++) {
			for (int z = 0; z < 100; z++) {
				if (y == 0)
					blocks.insert({ (x << 20) + (y << 10) + z, Block{ 1 } });
				else if ((x == 0 || x == 99 || z == 0 || z == 99) || (x % 9 == 0 && z % 5 == 0))
					blocks.insert({ (x << 20) + (y << 10) + z, Block{ 0 } });
				else if (y == 4 && x % 3 != 0 && z % 4 != 0)
					blocks.insert({ (x << 20) + (y << 10) + z, Block{ 3 } });
				else if (y == 9)
					blocks.insert({ (x << 20) + (y << 10) + z, Block{ 2 } });
				else if ((y == 2 || y == 6) && (x%11==0 && z%11==0)) {
					Light l;
					Dynamic d;
					d.pos.x = x+0.5f;
					d.pos.z = z+0.5f;
					d.pos.y = y+0.5f;
					d.textureID = 1;
					d.dlightIndex = lights.size();
					d.unlit = true;
					d.size.x = 0.01f; d.size.y = 0.03f;
					lights.push_back(l);
					dyn.push_back(d);
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
	dynTextures[1].loadFromFile("Projectile.png");

	for (unsigned int i = 0; i < 5; i++) {
		Dynamic d;
		d.textureID = 0;
		d.pos.x += i*0.5f;
		d.dir = sf::Vector3f((rand() % 40 - rand() % 20) * 0.01f, 0, (rand() % 40 - rand() % 20) * 0.01f);
		dyn.push_back(d);
	}

	cam.fovH *= PI / 180.0f;
	cam.fovV *= PI / 180.0f;
}

World::~World() {
}

void World::UpdateImage(sf::Image* v, short ystart, short yadd, short xstart, short xadd)
{
	float vStart = cam.fovV / 2;
	float vIncreaseBy = cam.fovV / height;
	float vOff = std::sin(cam.hrotation);
	float hStart = cam.rotation - cam.fovH/2;
	float hIncreaseBy = cam.fovH / width;
	float hrayAngle;
	float vrayAngle;
	Ray* r = &rays[ystart];
	for (int i = xstart; i < width; i+=xadd) {
		hrayAngle = (hStart + hIncreaseBy*i);
		r->angle = hrayAngle;
		r->dir.x = std::sin(hrayAngle); 
		r->dir.z = std::cos(hrayAngle);
		r->dir /= std::cos(cam.rotation - hrayAngle);		//Fix distortion on edges

		for (int j = ystart; j < height; j+=yadd) {
			vrayAngle = (vStart - j * vIncreaseBy);
			r->yscale = std::cos(cam.hrotation + vrayAngle);
			r->dir.y = (vOff + std::sin(vrayAngle));
			Raycast(r);
			v->setPixel(i,j,r->c);
		}
	}
}

void World::UpdateWorld()
{
	if (cam.velocity.x != 0 || cam.velocity.z != 0 || cam.velocity.y != 0 || cam.airtime>0) {
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
	float dirx = std::sin(cam.rotation);
	float dirz = std::cos(cam.rotation);
	cam.velocity = sf::Vector3f(cam.speed.x * dirx + cam.speed.z * dirz, cam.velocity.y, cam.speed.x * dirz - cam.speed.z * dirx);
	cam.velocity.x *= cam.speedM;
	cam.velocity.z *= cam.speedM;
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
	cam.velocity.y -= 0.002f;
	float xlimits[2] = { -0.1f,0.1f }; float ylimits[2] = { -0.8f,0.1f }; float zlimits[2] = { -0.1f,0.1f };
	sf::Vector3i testPos;
	for (unsigned int x = 0; x < 2; x++) {
		for (unsigned int y = 0; y < 2; y++) {
			for (unsigned int z = 0; z < 2; z++) {
				testPos.x = (int)(cam.pos.x + cam.velocity.x + xlimits[x]); testPos.y = (int)(cam.pos.y + ylimits[y]); testPos.z = (int)(cam.pos.z + zlimits[z]);
				if (blocks.contains((testPos.x<<20)+(testPos.y<<10)+testPos.z))
					cam.velocity.x = 0;
				testPos.x = (int)(cam.pos.x + xlimits[x]); testPos.y = (int)(cam.pos.y + cam.velocity.y + ylimits[y]);
				if (blocks.contains((testPos.x << 20) + (testPos.y << 10) + testPos.z))
					cam.velocity.y = 0;
				testPos.y = (int)(cam.pos.y + ylimits[y]); testPos.z = (int)(cam.pos.z + cam.velocity.z + zlimits[z]);
				if (blocks.contains((testPos.x << 20) + (testPos.y << 10) + testPos.z))
					cam.velocity.z = 0;
			}
		}
	}
	if (cam.velocity.x || cam.velocity.y || cam.velocity.z)
		cam.airtime = 0.2f;
	else if (cam.airtime > 0)
		cam.airtime -= 0.016f;		//TODO
	cam.pos += cam.velocity;
	//std::cout << cam.pos.x << "; " << cam.pos.y << "; " << cam.pos.z << "\n";
}

void World::Jump(float speed)
{
	cam.velocity.y = speed;
}

void World::Shoot()
{
	Dynamic d;
	Light l;
	d.dir = sf::Vector3f(std::sin(cam.rotation), std::sin(cam.hrotation), std::cos(cam.rotation))*2.0f;
	d.dir += cam.velocity * 2.0f;
	d.dir *= 0.5f;
	d.pos = cam.pos;
	d.pos.y -= 0.2f;
	d.size.x = 0.01f; d.size.y = 0.03f;
	d.aliveTime = 5;
	l.intensity = 0.2f;
	l.b = 0.5f;
	l.g = 0.5f;
	d.unlit = true;
	d.projectile = true;
	d.textureID = 1;
	d.dlightIndex = lights.size();
	lights.push_back(l);
	dyn.insert(dyn.begin(),d);
}

void World::UpdateDyn()
{
	for (unsigned int i = 0; i < dyn.size(); i++) {
		Dynamic* l = &dyn.at(i);
		l->pos += l->dir * 0.16f;
		if (l->dlightIndex >= 0) {
			lights.at(l->dlightIndex).pos = l->pos;
		}
		sf::Vector3i posi(l->pos+l->dir);
		if (l->projectile && blocks.contains((posi.x << 20) + (posi.y << 10) + posi.z))
			RemoveDyn(i);
		else if (blocks.contains((posi.x << 20) + (posi.y << 10) + posi.z) && (l->dir.x || l->dir.y || l->dir.z)) {
			float distx = l->pos.x - 0.5f - posi.x; float disty = l->pos.y - 0.5f - posi.y; float distz = l->pos.z - 0.5f - posi.z;
			if (distx<0.05f)
				l->dir.x *= -1;
			if (disty < 0.05f)
				l->dir.y *= -1;
			if (distz < 0.05f)
				l->dir.z *= -1;
		}
		if (l->aliveTime > 0) {
			l->aliveTime -= 0.016f;
			if (l->aliveTime <= 0)
				RemoveDyn(i);
		}
	}

	for (unsigned int i = 0; i < dyn.size(); i++) {
		Dynamic* l = &dyn.at(i);
		sf::Vector3i posi(l->pos);
		if (!l->unlit) {
			l->r = 0; l->g = 0; l->b = 0;
			for (unsigned int j = 0; j < alights.size(); j++) {
				if (alights.at(j)->intensity > 0) {
					float dist = VLengthS(l->pos - alights.at(j)->pos);
					dist = std::max(dist, 1.0f);
					float add = alights.at(j)->intensity / dist - dist * 0.001f;
					if (add > 0) {
						l->r += add * alights.at(j)->r;
						l->g += add * alights.at(j)->g;
						l->b += add * alights.at(j)->b;
					}
				}
			}
		}

		sf::Vector3f to = dyn.at(i).pos - cam.pos;
		dyn.at(i).distToCamera = std::sqrtf(to.x * to.x + to.z * to.z);

		if (i>0 && dyn.at(i).distToCamera < dyn.at(i-1).distToCamera) {
			std::iter_swap(dyn.begin() + i, dyn.begin() + i - 1);
		}
	}
	alights.clear();
	for (unsigned int i = 0; i < lights.size(); i++) {
		float alightDistance = viewDistance * viewDistance * 1.5f;
		float angle = std::abs(VAngleXZ(VNormalizeXZ(lights.at(i).pos - cam.pos), (sf::Vector3f(std::sin(cam.rotation), 0, std::cos(cam.rotation)))));	//Deactivate lights behind you more easily
		if (VLengthS(lights.at(i).pos - cam.pos) < alightDistance - angle / PI * alightDistance * 0.7f) {
			alights.push_back(&lights.at(i));
		}
	}
}

void World::RemoveDyn(unsigned int index)
{
	unsigned int lindex = dyn.at(index).dlightIndex;
	dyn.erase(dyn.begin() + index);
	if (lindex >= 0) {
		lights.erase(lights.begin() + lindex);
		for (unsigned int i = 0; i < dyn.size(); i++) {
			if (dyn.at(i).dlightIndex > lindex)
				dyn.at(i).dlightIndex--;
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
	return ang > PI ? ang - PI2 : ang < -PI ? ang + PI2 : ang;
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

inline float World::VLengthS(sf::Vector3f v)
{
	return v.x * v.x + v.y * v.y + v.z * v.z;
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
	float raySpeed = 0;
	unsigned short colRay = 0;
	unsigned int i = 0;
	unsigned int maxiter = viewDistance*1.5f;

	while (dist<viewDistance && i<maxiter) {
		float xray = (dirxadd + dirxsign * (pos.x - posi.x)) / dirxlen;
		float yray = (diryadd + dirysign * (pos.y - posi.y)) / dirylen;
		float zray = (dirzadd + dirzsign * (pos.z - posi.z)) / dirzlen;
		//Rayspeed matches what is needed to reach next block
		if (xray <= yray && xray <= zray) {
			raySpeed = xray;
			tryPos.x += dir.x * (raySpeed+0.002f);
			tryPos.y += dir.y * raySpeed;
			tryPos.z += dir.z * raySpeed;
			colRay = 1;
		}
		else if (yray <= xray && yray <= zray) {
			raySpeed = yray;
			tryPos.x += dir.x * raySpeed;
			tryPos.y += dir.y * (raySpeed+0.002f);
			tryPos.z += dir.z * raySpeed;
			colRay = 2;
		}
		else {
			raySpeed = zray;
			tryPos.x += dir.x * raySpeed;
			tryPos.y += dir.y * raySpeed;
			tryPos.z += dir.z * (raySpeed+0.002f);
			colRay = 3;
		}

		float tryDist = dist + raySpeed;

		while (DI<dyn.size() && tryDist >= dyn.at(DI).distToCamera) {			//Go through all dynamic objects that are there before hitting the next block
			Dynamic* d = &dyn.at(DI);
			raySpeed = d->distToCamera - dist;					//Reduce rayspeed to hit object
			dist = d->distToCamera;
			pos += dir * raySpeed;									//Set new position
			float to = d->pos.y-pos.y;						//Vector from ray pos to object pos (middle, y ignored)
			float sizey = d->size.y * r->yscale;
			if (std::abs(to) < sizey) {
				float ang = VAngleXZ(dir, VNormalizeXZ(d->pos - cam.pos)) * dist;
				sf::Image* tex = &dynTextures[d->textureID];
				sf::Vector2u tsize = tex->getSize();
				float xf = (0.5f + ang / PI * 0.5f / d->size.x);
				if (xf > 0 && xf < 1) {
					int x = xf * tsize.x;
					int y = (sizey + to) / sizey / 2 * (tsize.y);
					x = std::max(x, 0); y = std::max(y, 0);
					sf::Color c = tex->getPixel(x, y);
					if (c.a > 127) {				//Transparency check
						c.r = std::min(c.r * d->r, 255.0f); c.g = std::min(c.g * d->g, 255.0f); c.b = std::min(c.b * d->b, 255.0f);
						r->c = c;
						return;
					}
				}
			}
			DI++;
		}

		dist = tryDist;
		pos = tryPos;
		posi.x = pos.x; posi.y = pos.y; posi.z = pos.z;
		//if (pos.x < 0 || pos.y < 0 || pos.z < 0 || pos.x >= 30 || pos.y >= 10 || pos.z >= 30)	//Check out of bounds. Shouldn't be necessary if area is covered
		//	break;
		if (blocks.contains((posi.x<<20)+(posi.y<<10)+posi.z)) {								//Hit a block
			Block* block = &blocks[(posi.x << 20) + (posi.y << 10) + posi.z];
			sf::Color c;
			if (block->textureID < 0) {
				c = colors[-block->textureID];
			}
			else {		//Get color from texture with coordinates
				sf::Image* tex = &textures[block->textureID];
				sf::Vector2u tsize = tex->getSize();
				if (colRay==1) {
					c = tex->getPixel(tsize.x * (pos.z - posi.z), tsize.y * (pos.y - posi.y));
					pos.x += dirxsign * 0.01f;	//Get it off the wall
					dirysign = 0;
					dirzsign = 0;
				}
				else if (colRay==2) {
					c = tex->getPixel(tsize.x * (pos.x - posi.x), tsize.y * (pos.z - posi.z));
					pos.y += dirysign * 0.01f;
					dirxsign = 0;
					dirzsign = 0;
				}
				else {
					c = tex->getPixel(tsize.x * (pos.x - posi.x), tsize.y * (pos.y - posi.y));
					pos.z += dirzsign * 0.01f;
					dirxsign = 0;
					dirysign = 0;
				}
			}

			float litr = std::max(0.05f / dist - dist * 0.0001f, 0.0f);
			float litg = litr;
			float litb = litr;

			for (unsigned int j = 0; j < alights.size(); j++) {		//Lights
				dist = VLengthS(pos - alights.at(j)->pos);
				float add = (alights.at(j)->intensity / dist - dist * 0.002f);		//Distance based value
				if (add > 0) {
					sf::Vector3f newDir = alights.at(j)->pos - pos;
					add *= ((newDir.x * dirxsign + newDir.y * dirysign + newDir.z * dirzsign) * 0.7f + 0.3f);		//Angle multiplication
					if (add > 0) {
						if (alights.at(j)->shadows && tryDist < shadowDistance) {
							dist = VLength(newDir);
							newDir /= dist;
							r->maxDist = dist;
							r->dir = newDir;
							r->pos = pos;
							if (LRaycast(r)) {
								litr += add * alights.at(j)->r;
								litg += add * alights.at(j)->g;
								litb += add * alights.at(j)->b;
							}
							r->dir = dir;
						}
						else {
							litr += add * alights.at(j)->r;
							litg += add * alights.at(j)->g;
							litb += add * alights.at(j)->b;
						}
					}
				}
			}
			c.r = std::min(c.r * litr, 255.0f); c.g = std::min(c.g * litg, 255.0f); c.b = std::min(c.b * litb, 255.0f);
			r->c = c;
			return;
		}
		i += 1;
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
	float raySpeed = 0;
	unsigned int i = 0;
	unsigned int maxIter = std::max(maxDist*2,20.0f);

	for (unsigned int i = 0; i < maxIter && dist < maxDist; i++) {
		if (blocks.contains((posi.x << 20) + (posi.y << 10) + posi.z)) {
			return false;
		}
		float raySpeed = std::min({ (dirxadd + dirxsign * (pos.x - posi.x)) / dirxlen,		//Rayspeed matches what is needed to reach next block
									(diryadd + dirysign * (pos.y - posi.y)) / dirylen,
									(dirzadd + dirzsign * (pos.z - posi.z)) / dirzlen });
		raySpeed += 0.002f;											//Add a little on top so it doesn't fall short
		dist += raySpeed;
		pos += dir * raySpeed;
		posi.x = pos.x; posi.y = pos.y; posi.z = pos.z;

		//if (pos.x < 0 || pos.y < 0 || pos.z < 0 || pos.x >= 30 || pos.y >= 10 || pos.z >= 30)	//Check out of bounds. Shouldn't be necessary if area is covered
		//	break;
	}
	return dist >= maxDist;
}
