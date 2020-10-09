#pragma once

#include <SFML/Graphics.hpp>
#include <iostream>
#include <unordered_map>
constexpr auto PI = 3.1415926535f;
constexpr auto PI2 = 6.28318530718f;
constexpr auto PIH = 1.57079632679f;

struct Camera {
	sf::Vector3f pos = { 0, 0, 0 };
	float rotation = 0;
	float hrotation = 0;
	float fovH = 75;
	float fovV = 47;
	float airtime = 0.2f;
	float speedM = 0.05f;
	sf::Vector3f speed;
	sf::Vector3f velocity;
};

struct Sphere {
	sf::Vector3f pos = { 0,0,0 };
	float radius = 5.0f;
	short textureID = 0;
	sf::Color c = sf::Color::White;
};

struct Ray {
	sf::Vector3f dir;
	sf::Vector3f pos;
	float maxDist = 100;
	float angle = 0;
	float yscale = 1;
	sf::Color c;
};

class SphereWorld
{
public:
	void UpdateImage(sf::Image* v, short ystart, short yadd, short xstart, short xadd);
	void UpdateWorld();
	void Move(float forw, float right);
	void Turn(float angle);
	void LookUp(float angle);
	void Jump(float speed);
	void Shoot();
	int width = 320;
	int height = 180;
	Camera cam;
	SphereWorld();
	~SphereWorld();
private:
	std::vector<Sphere> spheres;
	void Move(float forw, float right, float up);
	float LoopAngle(float angle);
	sf::Vector2f VNormalize(sf::Vector2f v);
	float VLength(sf::Vector2f v);
	float VAngleXZ(sf::Vector3f a, sf::Vector3f b);
	sf::Vector3f VNormalize(sf::Vector3f v);
	sf::Vector3f VNormalizeXZ(sf::Vector3f v);
	float VLength(sf::Vector3f v);
	float VLengthXZ(sf::Vector3f v);
	float VLengthS(sf::Vector3f v);
	void Raycast(Ray* r);
	sf::Image* textures = new sf::Image[10];
	Ray* rays = new Ray[16];
	sf::Clock clock;
};

