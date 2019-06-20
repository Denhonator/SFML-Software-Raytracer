#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
#include <unordered_map>
constexpr auto PI = 3.1415926535f;
constexpr auto PI2 = 6.28318530718f;
constexpr auto PIH = 1.57079632679f;

struct Camera {
	sf::Vector3f pos = { 15.5f, 1.9f, 15.5f };
	float rotation = 0;
	float hrotation = 0;
	float fovH = 75;
	float fovV = 47;
	float airtime = 0.2f;
	float speedM = 0.05f;
	sf::Vector3f speed;
	sf::Vector3f velocity;
};

struct Block {
	short textureID = 0;
};

struct Dynamic {
	short textureID = -1;
	sf::Vector3f pos = { 12.5f, 1.45f, 18.5f };
	sf::Vector3f dir = { 0, 0, 0 };
	sf::Vector2f size = sf::Vector2f(0.15f, 0.45f);
	float distToCamera = 1000;
	float r = 1;
	float g = 1;
	float b = 1;
	bool unlit = false;
	bool projectile = false;
	float aliveTime = 0;
	short dlightIndex = -1;
};

struct Light {
	sf::Vector3f pos = { 17.5f, 2.0f, 17.5f };
	float intensity = 2;
	float r = 1;
	float g = 1;
	float b = 1;
	bool shadows = true;
};

struct Ray {
	sf::Vector3f dir;
	sf::Vector3f pos;
	float maxDist = 100;
	float angle = 0;
	float yscale = 1;
	sf::Color c;
};

class World {
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
	float shadowDistance = 16;
	float viewDistance = 24;
	World();
	~World();
private:
	std::unordered_map<int, Block> blocks;
	void Move(float forw, float right, float up);
	void UpdateDyn();
	void RemoveDyn(unsigned int index);
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
	bool LRaycast(Ray* r);
	sf::Color* colors = new sf::Color[10];
	sf::Image* textures = new sf::Image[10];
	sf::Image* dynTextures = new sf::Image[10];
	std::vector<Dynamic> dyn;
	std::vector<Light> lights;
	std::vector<Light*> alights;
	Ray* rays = new Ray[4];
	sf::Clock clock;
};