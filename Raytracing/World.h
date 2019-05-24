#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
#define PI 3.1415926535f

struct Camera {
	sf::Vector3f pos = { 50.5f, 1.5f, 50.5f };
	float rotation = 0;
	float fovH = 70;
	float fovV = 40;
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
	void Move(float forw, float right);
	int width = 320;
	int height = 180;
	World();
	~World();
private:
	float Cos(float angle);
	float Sin(float angle);
	sf::Color Raycast(sf::Vector3f ldir, float rayAngle);
	float* sines = new float[2160];
	unsigned int maxIter = 40;
	Camera cam;
	const int colorMults = 50;
	sf::Color* colors = new sf::Color[10];
	sf::Color* colorMult = new sf::Color[colorMults];
};