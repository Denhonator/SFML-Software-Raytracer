#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
#define PI 3.1415926535f

struct Camera {
	sf::Vector3f pos = { 50.5f, 1.5f, 50.5f };
	float rotation = 0;
	float hrotation = 0;
	float fovH = 70;
	float fovV = 70;
};

struct Block {
	short textureID = 0;
};

class World {
public:
	Block blocks[100][3][100];
	void UpdateScreenVertex(sf::VertexArray* v, int xoff, int yoff);
	void GetDir(float angle, sf::Vector3f* dir);
	void Turn(float angle);
	void LookUp(float angle);
	void Move(float forw, float right);
	int width = 320;
	int height = 180;
	World();
	~World();
private:
	float Cos(float angle);
	float Sin(float angle);
	float LoopAngle(float angle);
	sf::Color Raycast(sf::Vector3f ldir, float rayAngle);
	float* sines = new float[2160];
	unsigned int maxIter = 20;
	Camera cam;
	sf::Color* colors = new sf::Color[10];
	sf::Image* textures = new sf::Image[10];
};