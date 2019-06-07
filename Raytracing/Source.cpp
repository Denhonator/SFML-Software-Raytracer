#include "World.h"
#include <thread>

sf::Texture screenTexture;
sf::Image gameImage;
World world;
unsigned int lastIndex = 0;
unsigned int width = 640;		//Raycast + screen texture resolution
unsigned int height = 360;
int cyclesPerFrame = 1;
int fullCycles = 16;
bool run = true;
short draw[4] = { 0,0,0,0 };

void RenderThread(short num) {
	short cycle = 0;
	while (run) {
		if (draw[num]) {
			world.UpdateImage(&gameImage, num, 4, cycle, fullCycles);
			draw[num] -= 1;
			cycle = (cycle + 3) % fullCycles;
		}
		else
			sf::sleep(sf::Time(sf::milliseconds(1)));
	}
}

void main() {
	sf::Vector2i mousePos(0, 0);
	bool lockMouse = true;

	sf::Sprite screenSprite;
	screenTexture.create(1920, 1080);
	sf::RenderWindow window(sf::VideoMode(1920, 1080), "Rays");
	window.setVerticalSyncEnabled(true);
	world.width = width;
	world.height = height;

	screenSprite.setTexture(screenTexture);
	screenSprite.setScale(window.getSize().x / (float)width, window.getSize().y / (float)height);
	gameImage.create(1920,1080);

	std::thread p1 = std::thread(&RenderThread, 0);
	std::thread p2 = std::thread(&RenderThread, 1);
	std::thread p3 = std::thread(&RenderThread, 2);
	std::thread p4 = std::thread(&RenderThread, 3);

	sf::Clock clock;
	float frameTime;

	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();

			int sign = event.type == sf::Event::KeyPressed ? 1 : event.type == sf::Event::KeyReleased ? 0 : -1;
			if (sign>=0) {
				if (event.key.code == sf::Keyboard::A)
					world.Move(2, -1 * sign);
				else if (event.key.code == sf::Keyboard::D)
					world.Move(2, 1 * sign);
				else if (event.key.code == sf::Keyboard::W)
					world.Move(1 * sign, 2);
				else if (event.key.code == sf::Keyboard::S)
					world.Move(-1 * sign, 2);
				else if (event.key.code == sf::Keyboard::Space)
					world.Jump(0.05f * sign);
				else if (event.key.code == sf::Keyboard::LShift)
					world.cam.speedM = sign * 0.05f + 0.05f;
				else if (event.key.code == sf::Keyboard::Escape)
					lockMouse = false;
				else if (event.key.code == sf::Keyboard::PageDown && sign && height > 140) {
					width -= 16;
					height -= 9;
					world.width -= 16;
					world.height -= 9;
					screenSprite.setScale(window.getSize().x / (float)width, window.getSize().y / (float)height);
				}
				else if (event.key.code == sf::Keyboard::PageUp && sign && height+9<=window.getSize().y) {
					width += 16;
					height += 9;
					world.width += 16;
					world.height += 9;
					screenSprite.setScale(window.getSize().x / (float)width, window.getSize().y / (float)height);
				}
			}

			else if (event.type == sf::Event::MouseButtonPressed) {
				if (event.mouseButton.button == sf::Mouse::Button::Left)
					world.Shoot();
			}

			else if (event.type == sf::Event::MouseMoved && lockMouse) {
				world.Turn((event.mouseMove.x - mousePos.x) * 0.01f);
				world.LookUp((mousePos.y - event.mouseMove.y) * 0.01f);
				mousePos = sf::Vector2i(event.mouseMove.x, event.mouseMove.y);
			}
		}

		if (event.type == sf::Event::MouseButtonPressed) {
			lockMouse = true;
		}

		if (event.type == sf::Event::LostFocus) {
			lockMouse = false;
		}

		window.setMouseCursorGrabbed(lockMouse);
		window.setMouseCursorVisible(!lockMouse);
		if (lockMouse) {
			sf::Mouse::setPosition(sf::Vector2i(width / 2, height / 2), window);
			mousePos.x = width / 2; mousePos.y = height / 2;
		}

		clock.restart();

		world.UpdateWorld();

		draw[0] = cyclesPerFrame; draw[1] = cyclesPerFrame; draw[2] = cyclesPerFrame; draw[3] = cyclesPerFrame;	//Draw in 4 threads here and only here
		while (draw[0] || draw[1] || draw[2] || draw[3])
			sf::sleep(sf::milliseconds(1));
		frameTime = clock.getElapsedTime().asSeconds();

		screenTexture.loadFromImage(gameImage);
		window.draw(screenSprite);
		window.display();

		if (frameTime >= 0.021f) {		//Too slow
			cyclesPerFrame = std::max(cyclesPerFrame - 1, 1);
		}
		if (frameTime <= 0.01f) {		//Pretty fast
			cyclesPerFrame = std::min(cyclesPerFrame + 1, fullCycles);
		}
		std::cout << width << "; " << height << "\n" << cyclesPerFrame << "\n";
	}
	run = false;
	p1.join();
	p2.join();
	p3.join();
	p4.join();
}