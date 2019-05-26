#include "World.h"
#include <thread>

sf::VertexArray screenVertex;
sf::RenderTexture screenTexture;
World world;
unsigned int lastIndex = 0;
const unsigned int width = 640;		//Raycast + screen texture resolution
const unsigned int height = 360;
const unsigned int cyclesPerFrame = 2;
bool run = true;
short draw[4] = { 0,0,0,0 };

void RenderThread(int num) {
	short cycle = 0;
	while (run) {
		if (draw[num]) {
			world.UpdateScreenVertex(&screenVertex, num, cycle);
			draw[num] -= 1;
			cycle = (cycle + 1) % 4;
		}
		sf::sleep(sf::Time(sf::milliseconds(1)));
	}
}

void main() {
	sf::Sprite screenSprite;
	screenTexture.create(width, height, false);
	sf::RenderWindow window(sf::VideoMode(1920, 1080), "Rays");
	window.setVerticalSyncEnabled(true);
	world.width = width;
	world.height = height;

	screenSprite.setTexture(screenTexture.getTexture());
	screenSprite.setScale(window.getSize().x / (float)width, window.getSize().y / (float)height);
	screenVertex.resize(width * height);
	for (unsigned int i = 0; i < width; i++) {
		for (unsigned int j = 0; j < height; j++) {
			unsigned int index = i + width * j;
			screenVertex[index].position = sf::Vector2f(i, j+1);
		}
	}

	std::thread p1 = std::thread(&RenderThread, 0);
	std::thread p2 = std::thread(&RenderThread, 1);
	std::thread p3 = std::thread(&RenderThread, 2);
	std::thread p4 = std::thread(&RenderThread, 3);

	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
		}

		float speed = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) ? 0.08f : 0.04;
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
			world.Move(0,-speed,0);
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
			world.Move(0, speed,0);
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
			world.Move(speed,0,0);
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
			world.Move(-speed,0,0);
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
			world.Jump(0.05f);
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl))
			world.Move(0, 0, -speed);
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
			world.Turn(-speed * 20);
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
			world.Turn(speed * 20);
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
			world.LookUp(speed * 20);
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
			world.LookUp(-speed * 20);

		world.UpdateWorld();

		draw[0] = cyclesPerFrame; draw[1] = cyclesPerFrame; draw[2] = cyclesPerFrame; draw[3] = cyclesPerFrame;	//Draw in 4 threads here and only here
		while (draw[0] || draw[1] || draw[2] || draw[3])
			sf::sleep(sf::milliseconds(1));

		screenTexture.draw(screenVertex);
		screenTexture.display();
		window.clear();
		window.draw(screenSprite);
		window.display();
	}
	run = false;
	p1.join();
	p2.join();
	p3.join();
	p4.join();
}