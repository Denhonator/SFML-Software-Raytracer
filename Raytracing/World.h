#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
#define PI 3.1415926535f

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
	short textureID = 0;
	sf::Vector3f pos = { 15.5f, 4.3f, 16.0f };
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
	void UpdateDyn(int index = -1);
	float Cos(float angle);
	float Sin(float angle);
	float LoopAngle(float angle);
	float VAngle(sf::Vector2f a, sf::Vector2f b);
	sf::Vector2f VNormalize(sf::Vector2f v);
	float VLength(sf::Vector2f v);
	float VAngleXZ(sf::Vector3f a, sf::Vector3f b);
	sf::Vector3f VNormalize(sf::Vector3f v);
	sf::Vector3f VNormalizeXZ(sf::Vector3f v);
	float VLength(sf::Vector3f v);
	float VLengthXZ(sf::Vector3f v);
	void Raycast(Ray* r);
	float* sines = new float[2160];
	unsigned int maxIter = 15;
	Camera cam;
	sf::Color* colors = new sf::Color[10];
	sf::Image* textures = new sf::Image[10];
	sf::Image* dynTextures = new sf::Image[10];
	Dynamic* dyn = new Dynamic[10];
	Ray* rays = new Ray[4];
	sf::Clock clock;
};