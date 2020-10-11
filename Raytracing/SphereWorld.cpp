#include "SphereWorld.h"

float dot(sf::Vector3f a, sf::Vector3f b) {
	float product = a.x * a.x + a.y * a.y + a.z * a.z;
	return product;
}

sf::Vector3f cross(sf::Vector3f a, sf::Vector3f b) {
	sf::Vector3f t;
	t.x = a.y * b.z - a.z * b.y;
	t.y = a.x * b.z - a.z * b.x;
	t.z = a.x * b.y - a.y * b.x;
	return t;
}

sf::Vector3f QToDir(sf::Vector3f u, float s, sf::Vector3f v)
{
	return 2.0f * dot(u, v) * u
		+ (s * s - dot(u, u)) * v
		+ 2.0f * s * cross(u, v);
}

sf::Vector3f VRotate(sf::Vector3f v, sf::Vector3f k, float cos_theta, float sin_theta) {
	return (v * cos_theta) + (cross(k, v) * sin_theta) + (k * (float)dot(k, v)) * (1 - cos_theta);
}

sf::Vector3f VRotateX(sf::Vector3f v, float amount) {
	float s = std::sin(amount);
	float c = std::cos(amount);
	return sf::Vector3f(v.x, v.y * c - v.z * s, v.y * s + v.z * c);
}
sf::Vector3f VRotateY(sf::Vector3f v, float amount) {
	float s = std::sin(amount);
	float c = std::cos(amount);
	return sf::Vector3f(v.x * c + v.z * s, v.y, -v.x * s + v.z * c);
}
sf::Vector3f VRotateZ(sf::Vector3f v, float amount) {
	float s = std::sin(amount);
	float c = std::cos(amount);
	return sf::Vector3f(v.x * c - v.y * s, v.x * s + v.y * c, v.z);
}

SphereWorld::SphereWorld()
{
	srand(time(NULL));

	if (!shader.loadFromFile("rayShader.frag", sf::Shader::Fragment))
	{
		std::cout << "Failed to load shader\n";
	}

	textures[0].loadFromFile("Floor.png");
	stextures[0].loadFromFile("Floor.png");

	stextures[0].setRepeated(true);
	shader.setUniform("ground", stextures[0]);

	AddSphere(sf::Vector3f(0, 0, 0), 4);
	for (int i = 0; i < 10; i++) {
		AddSphere(sf::Vector3f((rand() % 20 - 10), (rand() % 20 - 10), (rand() % 20 - 10)), (rand() % 5 + 2));
	}

	cam.fovH *= PI / 180.0f;
	cam.fovV *= PI / 180.0f;

	//sf::Vector3f test = QToDir(sf::Vector3f(std::sin(PI / 8), 0, 0), std::cos(PI / 8), sf::Vector3f(0.707f,0,0.707f));
	//std::cout << test.x << ", " << test.y << ", " << test.z << std::endl;
}

SphereWorld::~SphereWorld()
{
}

void SphereWorld::UpdateImage(sf::Image* v, short ystart, short yadd, short xstart, short xadd)
{
	float vStart = -cam.fovV;
	float vIncreaseBy = cam.fovV / height * 2;
	float vOff = std::sin(cam.hrotation);
	float hStart =  - cam.fovH;
	float hIncreaseBy = cam.fovH / width * 2;
	float hrayAngle;
	float vrayAngle;
	Ray* r = &rays[ystart];

	for (int i = xstart; i < width; i += xadd) {
		hrayAngle = (hStart + hIncreaseBy * i);
		//r->angle = hrayAngle;
		/*r->dir.x = std::sin(hrayAngle);
		r->dir.z = std::cos(hrayAngle);*/
		//r->dir /= std::cos(cam.rotation - hrayAngle);		//Fix distortion on edges

		for (int j = ystart; j < height; j += yadd) {

			//sf::Vector3f start = sf::Vector3f((i / width - 0.5f) * s * aspectRatio, (j / height - 0.5f) * s, 1.0f) * 0.3f;
			//return Ray(start, start.Normalized());
			//r->dir = start - cam.pos;

			vrayAngle = (vStart + j * vIncreaseBy);
			/*r->yscale = std::cos(cam.hrotation + vrayAngle);
			r->dir.y = (std::sin(vrayAngle+cam.hrotation));*/
			//vrayAngle += cam.hrotation;

			sf::Vector3f up = VRotateX(sf::Vector3f(0,-1,0), -cam.hrotation);
			sf::Vector3f forward = VRotateX(sf::Vector3f(0,0,1), -cam.hrotation);
			sf::Vector3f right = VRotateY(sf::Vector3f(1,0,0), cam.rotation);
			forward = VRotateY(forward, cam.rotation);
			up = VRotateY(up, cam.rotation);

			r->dir = forward + right * hrayAngle + up * vrayAngle;
			//r->dir = QToDir(sf::Vector3f(0, std::sin(hrayAngle / 2), 0), std::cos(hrayAngle / 2), sf::Vector3f(0,0,1));
			//r->dir = VRotate(sf::Vector3f(0, 0, 1), sf::Vector3f(0, 1, 0), std::cos(hrayAngle), std::sin(hrayAngle));
			//r->dir = VRotate(r->dir, sf::Vector3f(1, 0, 0), std::cos(vrayAngle), std::sin(vrayAngle));
			//if (i == width / 2 && j == height / 2)
			//	std::cout << r->dir.x << ", " << r->dir.z << std::endl;
			//r->dir = QToDir(sf::Vector3f(std::sin(vrayAngle / 2), 0, 0), std::cos(vrayAngle / 2), r->dir);

			//r->dir = sf::Vector3f(std::sin(hrayAngle), 0, std::cos(hrayAngle));
			//r->dir = QToDir(sf::Vector3f(0, std::sin(-cam.rotation / 2), 0), std::cos(-cam.rotation / 2), r->dir);
			//r->dir = QToDir(sf::Vector3f(std::sin(vrayAngle / 2), 0, 0), std::cos(hrayAngle / 2), r->dir);
			//r->dir += QToDir(sf::Vector3f(0, PI / 2, 0), PI / 2, r->dir) * m_tanf * m_aspect * (float)i;
			//r->dir += QToDir(sf::Vector3f(PI / 2, 0, 0), PI / 2, r->dir) * m_tanf * (float)j;

			Raycast(r);
			v->setPixel(i, j, r->c);
		}
	}
}

void SphereWorld::UpdateWorld()
{
	if (cam.velocity.x != 0 || cam.velocity.z != 0 || cam.velocity.y != 0 || cam.airtime > 0) {
		Move(cam.speed.x, cam.speed.z, cam.speed.y);
	}
}

void SphereWorld::Move(float forw, float right)
{
	if (forw < 2)
		cam.speed.x = forw;
	if (right < 2)
		cam.speed.z = right;
}

void SphereWorld::Turn(float angle)
{
	cam.rotation += angle * cam.speedM;
	cam.rotation = LoopAngle(cam.rotation);
	float dirx = std::sin(cam.rotation);
	float dirz = std::cos(cam.rotation);
	cam.velocity = sf::Vector3f(cam.speed.x * dirx + cam.speed.z * dirz, cam.velocity.y, cam.speed.x * dirz - cam.speed.z * dirx);
	cam.velocity.x *= cam.speedM;
	cam.velocity.z *= cam.speedM;
	//std::cout << cam.rotation << "\n";
}

void SphereWorld::LookUp(float angle)
{
	angle *= cam.speedM;
	if (std::abs(cam.hrotation + angle) + cam.fovV / 2 < PIH)
		cam.hrotation += angle;
	//std::cout << cam.hrotation / PI * 180 << "\n";
}

void SphereWorld::AddSphere(sf::Vector3f pos, float radius)
{
	spheres.push_back(Sphere{ pos, radius });
	for (int i = 0; i < spheres.size(); i++) {
		bool removed = false;
		for (int j = 0; j < spheres.size() && !removed; j++) {
			if (i!=j && VLength(spheres.at(i).pos - spheres.at(j).pos) + spheres.at(i).radius <= spheres.at(j).radius) {
				spheres.erase(spheres.begin() + i);
				i--;
				removed = true;
			}
		}
		if(!removed)
			shader.setUniform("spheres[" + std::to_string(i) + "]", 
				sf::Glsl::Vec4(spheres.at(i).pos.x, spheres.at(i).pos.y, spheres.at(i).pos.z, spheres.at(i).radius));
	}
	shader.setUniform("sphereCount", (int)spheres.size());
}

void SphereWorld::Move(float forw, float right, float up)
{
	cam.velocity.y -= 0.006f;
	float xlimits[2] = { -0.1f,0.1f }; float ylimits[2] = { -0.8f,0.3f }; float zlimits[2] = { -0.1f,0.1f };
	sf::Vector3f testPos;
	for (unsigned int x = 0; x < 2; x++) {
		for (unsigned int y = 0; y < 2; y++) {
			for (unsigned int z = 0; z < 2; z++) {
				testPos.x = (cam.pos.x + cam.velocity.x + xlimits[x]); testPos.y = (cam.pos.y + ylimits[y]); testPos.z = (cam.pos.z + zlimits[z]);
				bool isInside = false;
				for (int i = 0; i < spheres.size() && !isInside; i++) {
					if (VLength(testPos - spheres.at(i).pos) < spheres.at(i).radius)
						isInside = true;
				}
				if(!isInside)
					cam.velocity.x = 0;

				testPos.x = (cam.pos.x + xlimits[x]); testPos.y = (cam.pos.y + cam.velocity.y + ylimits[y] - 0.2f);
				isInside = false;
				for (int i = 0; i < spheres.size() && !isInside; i++) {
					if (VLength(testPos - spheres.at(i).pos) < spheres.at(i).radius)
						isInside = true;
				}
				if (!isInside)
					cam.velocity.y = 0;

				testPos.y = (cam.pos.y + ylimits[y]); testPos.z = (cam.pos.z + cam.velocity.z + zlimits[z]);
				isInside = false;
				for (int i = 0; i < spheres.size() && !isInside; i++) {
					if (VLength(testPos - spheres.at(i).pos) < spheres.at(i).radius)
						isInside = true;
				}
				if (!isInside)
					cam.velocity.z = 0;
			}
		}
	}
	cam.onGround = false;
	for (int i = 0; i < spheres.size(); i++) {
		rays[0].dir = sf::Vector3f(0, -1, 0);
		rays[1].dir = sf::Vector3f(0, 1, 0);
		Raycast(rays);
		Raycast(&rays[1]);
		if (cam.velocity.y <= 0 && (cam.pos.y - rays[0].pos.y) <= 1.0f) {
			cam.pos.y = 1.0f + rays[0].pos.y;
		}
		if ((cam.pos.y - rays[0].pos.y) <= 1.4f)
			cam.onGround = true;
	}

	if (cam.velocity.x || cam.velocity.y || cam.velocity.z)
		cam.airtime = 0.2f;
	else if (cam.airtime > 0)
		cam.airtime -= 0.016f;		//TODO
	cam.pos += cam.velocity;
	//std::cout << cam.pos.x << "; " << cam.pos.y << "; " << cam.pos.z << "\n";
}

void SphereWorld::Jump(float speed)
{
	if(cam.onGround)
		cam.velocity.y = speed * 2;
}

void SphereWorld::Shoot()
{
}

inline float SphereWorld::LoopAngle(float angle)
{
	return angle > PI2 ? angle - PI2 : (angle < 0 ? angle + PI2 : angle);
}

inline sf::Vector2f SphereWorld::VNormalize(sf::Vector2f v)
{
	return v / VLength(v);
}

inline float SphereWorld::VLength(sf::Vector2f v)
{
	return std::sqrtf(v.x * v.x + v.y * v.y);
}

inline float SphereWorld::VAngleXZ(sf::Vector3f a, sf::Vector3f b)
{
	float ang = std::atan2(b.z, b.x) - std::atan2(a.z, a.x);
	return ang > PI ? ang - PI2 : ang < -PI ? ang + PI2 : ang;
}

inline sf::Vector3f SphereWorld::VNormalize(sf::Vector3f v)
{
	return v / VLength(v);
}

inline sf::Vector3f SphereWorld::VNormalizeXZ(sf::Vector3f v)
{
	return v / VLengthXZ(v);
}

inline float SphereWorld::VLength(sf::Vector3f v)
{
	return std::sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

inline float SphereWorld::VLengthXZ(sf::Vector3f v)
{
	return std::sqrtf(v.x * v.x + v.z * v.z);
}

inline float SphereWorld::VLengthS(sf::Vector3f v)
{
	return v.x * v.x + v.y * v.y + v.z * v.z;
}

void SphereWorld::Raycast(Ray* r)
{
	r->pos = cam.pos;
	r->dir = VNormalize(r->dir);
	float largestDist = 1;
	int drawSphere = 0;

	while (largestDist>0) {
		largestDist = 0;
		for (int i = 0; i < spheres.size(); i++) {
			float dist = VLength(r->pos - spheres.at(i).pos);
			if (spheres.at(i).radius - dist > 0.01f) {
				largestDist = std::max(largestDist, spheres.at(i).radius - dist);
				drawSphere = i;
			}
		}
		r->pos += r->dir * largestDist;
	}
	float xcoord = VAngleXZ(r->pos, spheres.at(drawSphere).pos)/PI2 + 1.0f;
	float ycoord = (std::asinf(VNormalize(r->pos - spheres.at(drawSphere).pos).y) / PI + 0.5f);
	float brightness = 3.0f / std::max(VLength(r->pos - cam.pos), 3.0f);
	sf::Vector2u texsize = textures[spheres.at(drawSphere).textureID].getSize();
	sf::Color c = textures[spheres.at(drawSphere).textureID].getPixel(std::fmodf(xcoord*4*spheres.at(drawSphere).radius, 1.0f) * texsize.x, std::fmodf(ycoord*2*spheres.at(drawSphere).radius, 1.0f) * texsize.y);
	c.r *= brightness;
	c.g *= brightness;
	c.b *= brightness;
	r->c = c;
}