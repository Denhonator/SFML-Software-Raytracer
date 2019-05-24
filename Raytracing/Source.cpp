#include "World.h"
#include <thread>

sf::VertexArray screenVertex;
sf::RenderTexture screenTexture;
World world;
unsigned int lastIndex = 0;
const unsigned int width = 320;
const unsigned int height = 180;
bool run = true;

void RenderThread(int xoff, int yoff) {
	while (run) {
		world.UpdateScreenVertex(&screenVertex, xoff, yoff);
		sf::sleep(sf::Time(sf::milliseconds(15)));
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
	screenSprite.setScale(window.getSize().x / width, window.getSize().y / height);
	screenVertex.resize(width * height);
	for (unsigned int i = 0; i < width; i++) {
		for (unsigned int j = 0; j < height; j++) {
			unsigned int index = i + width * j;
			screenVertex[index].position = sf::Vector2f(i, j);
		}
	}

	std::thread p1 = std::thread(&RenderThread, 0, 0);
	std::thread p2 = std::thread(&RenderThread, 0, 1);
	std::thread p3 = std::thread(&RenderThread, 1, 0);
	std::thread p4 = std::thread(&RenderThread, 1, 1);

	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
		}

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
			world.Move(0,-0.05f);
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
			world.Move(0,0.05f);
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
			world.Move(0.05f,0);
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
			world.Move(-0.05f,0);
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
			world.Turn(-2);
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
			world.Turn(2);

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