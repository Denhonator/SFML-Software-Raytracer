#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
#define PI 3.1415926535f
#define PI2 6.28318530718f
#define PIH 1.57079632679f

struct Camera {
	sf::Vector3f pos = { 15.5f, 1.5f, 15.5f };
	float rotation = 0;
	float hrotation = 0;
	float fovH = 75;
	float fovV = 47;
	float ySpeed = 0;
};

struct Block {
	short textureID = 0;
};

struct Dynamic {
	short textureID = -1;
	sf::Vector3f pos = { 15.5f, 4.6f, 16.0f };
	sf::Vector2f size = sf::Vector2f(0.1f, 0.3f);
	float distToCamera = 1000;
};

struct Ray {
	sf::Vector3f dir;
	float angle = 0;
	sf::Color c;
};

class World {
public:
	Block blocks[30][10][30];
	void UpdateScreenVertex(sf::VertexArray* v, short num, short cycle);
	void UpdateWorld();
	void Turn(float angle);
	void LookUp(float angle);
	void Move(float forw, float right, float up);
	void Jump(float speed);
	int width = 320;
	int height = 180;
	World();
	~World();
private:
	void DynMove(unsigned int index, sf::Vector3f dir);
	void UpdateDyn();
	float LoopAngle(float angle);
	sf::Vector2f VNormalize(sf::Vector2f v);
	float VLength(sf::Vector2f v);
	float VAngleXZ(sf::Vector3f a, sf::Vector3f b);
	sf::Vector3f VNormalize(sf::Vector3f v);
	sf::Vector3f VNormalizeXZ(sf::Vector3f v);
	float VLength(sf::Vector3f v);
	float VLengthXZ(sf::Vector3f v);
	void Raycast(Ray* r);
	unsigned int maxIter = 15;
	Camera cam;
	sf::Color* colors = new sf::Color[10];
	sf::Image* textures = new sf::Image[10];
	sf::Image* dynTextures = new sf::Image[10];
	Dynamic* dyn = new Dynamic[10];
	Ray* rays = new Ray[4];
	sf::Clock clock;
};