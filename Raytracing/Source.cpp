//#include "World.h"
#include "SphereWorld.h"
#include <thread>

sf::Texture screenTexture;
sf::Image gameImage;
SphereWorld world;
unsigned int lastIndex = 0;
unsigned int width = 640;		//Raycast + screen texture resolution
unsigned int height = 360;
int cyclesPerFrame = 1;
short fullCycles = 4;
const int threadCount = 8;
bool run = true;
short draw[threadCount];

void RenderThread(short num) {
	short cycle = 0;
	while (run) {
		if (draw[num]) {
			world.UpdateImage(&gameImage, num, threadCount, cycle, fullCycles);
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
	sf::RenderTexture rt;
	rt.create(640, 360);
	window.setVerticalSyncEnabled(60);
	world.width = width;
	world.height = height;

	screenSprite.setTexture(screenTexture);
	screenSprite.setScale(window.getSize().x / (float)width, window.getSize().y / (float)height);
	gameImage.create(1920,1080);

	std::thread threads[threadCount];

	for (unsigned int i = 0; i < threadCount; i++) {
		draw[i] = 0;
		threads[i] = std::thread(&RenderThread, i);
	}

	sf::Clock clock;
	float frameTime;

	while (run)
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				run = false;

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
					world.Jump(0.1f * sign);
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

		if (!run)
			break;

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

		/*for (unsigned int i = 0; i < threadCount; i++) {
			draw[i] = cyclesPerFrame;
		}
		for (unsigned int i = 0; i < threadCount; i+=0) {
			if (!draw[i])
				i += 1;
			else
				sf::sleep(sf::milliseconds(1));
		}*/

		//screenTexture.loadFromImage(gameImage);

		world.shader.setUniform("campos", world.cam.pos);
		world.shader.setUniform("rotation", sf::Vector2f(world.cam.rotation, world.cam.hrotation));
		world.shader.setUniform("fov", sf::Vector2f(world.cam.fovH, world.cam.fovV));
		world.shader.setUniform("size", sf::Vector2f(rt.getSize()));
		//world.shader.setUniform("texture", screenTexture);
		//screenSprite.setTexture(rt.getTexture());

		sf::Sprite sp(rt.getTexture());

		rt.draw(sp, &world.shader);
		rt.display();

		window.setView(sf::View(sf::FloatRect(0, 0, rt.getSize().y*16.0f/9.0f, rt.getSize().y)));
		window.draw(sp);
		window.display();

		frameTime = clock.getElapsedTime().asSeconds();

		if (frameTime >= 0.021f) {		//Too slow
			cyclesPerFrame = std::max(cyclesPerFrame - 1, 1);
		}
		if (frameTime <= 0.01f) {		//Pretty fast
			cyclesPerFrame = std::min(cyclesPerFrame + 1, (int)fullCycles);
		}
		//std::cout << 1.0f/frameTime << "\n";
	}
	run = false;
	for (unsigned int i = 0; i < threadCount; i++) {
		draw[i] = 0;
	}
	for (unsigned int i = 0; i < threadCount; i++) {
		threads[i].join();
	}
}