#include "World.h"
#include <thread>

sf::VertexArray screenVertex;
sf::RenderTexture screenTexture;
World world;
unsigned int lastIndex = 0;
const unsigned int width = 320;		//Raycast + screen texture resolution
const unsigned int height = 180;
bool run = true;
bool draw[4] = { false,false,false,false };

void RenderThread(int num) {
	while (run) {
		if (draw[num]) {
			world.UpdateScreenVertex(&screenVertex, num & 2 ? 1 : 0, num & 1 ? 1 : 0);
			draw[num] = false;
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
			world.Move(0,-speed);
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
			world.Move(0, speed);
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
			world.Move(speed,0);
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
			world.Move(-speed,0);
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
			world.Turn(-speed * 20);
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
			world.Turn(speed * 20);
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
			world.LookUp(speed * 30);
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
			world.LookUp(-speed * 30);

		draw[0] = true; draw[1] = true; draw[2] = true; draw[3] = true;	//Draw in 4 threads here and only here
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