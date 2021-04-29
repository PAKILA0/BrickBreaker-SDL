#include <iostream>
#include <string>
#include <SDL.h>
#include <SDL_ttf.h>
#include <list>
/*
	Ball The Game - Midterm Project for Computer Graphic - 2020/12/18 <- DEAD LINE
	by Allen Checn 
	This is a simple brick breaker game using SDL, require SDL and SDL_ttf
	You can download the package via NuGet if you are using Visual Studio 2019
*/


//Screen Sizes - Define The Screen Size
#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 640

//Texts Setting - Text Sizes
#define FONT_SIZE 32

//Ball & Paddle Setting
#define BALL_SPEED 9
#define PADDLE_SPEED 9
#define BALL_SIZE 16
#define TRAIL_SIZE 6

//Bricks Setting
#define COL 13
#define ROW 8
#define SPACING 16

//Utility
#define PI 3.14159265358979323846

//SDL Objects - Initialize objects 
SDL_Renderer* renderer;
SDL_Window* window;
TTF_Font* font;
SDL_Color color, brickColor;

//Define UI Blocks & Object
SDL_Rect paddle, ball, lives, brick, trail;

//Global Stats
int chapter;

int ballSize = BALL_SIZE;
int ballSpeed = BALL_SPEED;

int coloms = COL;
int rows = ROW;

int points;

bool running;
int frameCount, timerFPS, lastFrame, fps;
float velocityY, velocityX;
int livesCount;

int bricks[ROW * COL]; // health points of each bricks

std::list<float> trailCacheX;
std::list<float> trailCacheY;


//This Function Reset Everything Back to Initial State
void resetGame(bool toOne) {
	if (toOne) {
		chapter = 1;
		ballSpeed = BALL_SPEED;
	}
	else {//Load Next Level
		chapter++;
		ballSpeed += 2;
	}


	for (int i = 0; i < coloms * rows; i++) {
		bricks[i] = chapter;
	}

	points = 0;
	livesCount = 3;
	paddle.x = (SCREEN_WIDTH / 2) - (paddle.w / 2);
	ball.y = paddle.y - (paddle.h * 4);
	velocityY = ballSpeed / 2;
	velocityX = 0;
	ball.x = SCREEN_WIDTH / 2 - (ballSize / 2);
}

//Set the position of BrickRect
void setBrick(int i) {
	brick.x = ((((i % coloms) + 1) * SPACING) + ((i % coloms) * brick.w) - (SPACING / 2));
	brick.y = (brick.h * 3 + (((i % rows) + 1) * SPACING) + ((i % rows) * brick.h) - (SPACING / 2)) + 30 * chapter;
}

//Display info on the screen with ttf module
void write(std::string text, int x, int y) {
	SDL_Surface* surface;
	SDL_Texture* texture;

	const char* t = text.c_str();

	surface = TTF_RenderText_Solid(font, t, color);
	texture = SDL_CreateTextureFromSurface(renderer, surface);

	lives.w = surface->w;
	lives.h = surface->h;
	lives.x = x - lives.w;
	lives.y = y - lives.h;

	SDL_FreeSurface(surface);
	SDL_RenderCopy(renderer, texture, NULL, &lives);
	SDL_DestroyTexture(texture);
}

//Main update loop for the game logic
void update() {

	//Behavior When Ball is Interacting With The Paddle
	if (SDL_HasIntersection(&ball, &paddle)) {
		float rel = (paddle.x + (paddle.w /2)) - (ball.x + (ballSize /2));
		float norm = rel / (paddle.w / 2);
		float bounce = norm * (5 * PI / 12);

		velocityY = -ballSpeed * cos(bounce);
		velocityX = ballSpeed * -sin(bounce);
	}

	if (ball.y <= 0) { velocityY = -velocityY; }
	if (ball.y + ballSize >= SCREEN_HEIGHT) { velocityY = -velocityY; livesCount--; }
	if (ball.x <= 0 || ball.x + ballSize >= SCREEN_WIDTH) velocityX = -velocityX;
	ball.y += velocityY;
	ball.x += velocityX;
	if (paddle.x < 0)paddle.x = 0;
	if (paddle.x + paddle.w > SCREEN_WIDTH)paddle.x = SCREEN_WIDTH - paddle.w;
	if (livesCount <= 0)resetGame(true);

	//Behavior when balls is interacting with the bricks
	for (int i = 0; i < coloms * rows; i++) {
		setBrick(i);
		if (SDL_HasIntersection(&ball, &brick) && bricks[i] > 0) {
			points++;
			bricks[i]--;

			if (ball.x >= brick.x) { velocityX = velocityX * -1; ball.x = ball.x - 20; }
			if (ball.x <= brick.x) { velocityX = velocityX * -1; ball.x = ball.x + 20; }
			if (ball.y <= brick.y) { velocityY = velocityY * -1; ball.y = ball.y - 20; }
			if (ball.y >= brick.y) { velocityY = velocityY * -1; ball.y = ball.y + 20; }

			if (points == 5 * chapter) {
				resetGame(false);
			}
		}
	}
}

//Get input and functions
void input() {
	SDL_Event e;
	const Uint8* keystates = SDL_GetKeyboardState(NULL);
	while (SDL_PollEvent(&e)) if (e.type == SDL_QUIT) running = false;
	if (keystates[SDL_SCANCODE_ESCAPE]) running = false;
	if (keystates[SDL_SCANCODE_LEFT]) paddle.x -= PADDLE_SPEED;
	if (keystates[SDL_SCANCODE_RIGHT]) paddle.x += PADDLE_SPEED;
	if (keystates[SDL_SCANCODE_UP]) { ballSize++; ball.w = ball.h = ballSize; trail.w = trail.h = ballSize;}
	if (keystates[SDL_SCANCODE_DOWN]) { ballSize--; ball.w = ball.h = ballSize; trail.w = trail.h = ballSize;}
}

//Draw the things
void render() {
	SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 255);
	SDL_RenderClear(renderer);

	//Time control
	frameCount++;
	timerFPS = SDL_GetTicks() - lastFrame;
	if (timerFPS < (1000 / 60)) {
		SDL_Delay((1000 / 60) - timerFPS);
	}

	//White for paddles
	SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
	SDL_RenderFillRect(renderer, &paddle);
	SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
	SDL_RenderFillRect(renderer, &ball);

	//Trail - Cache last 6 position of the ball and render them with degraded alpha value
	#pragma region Trailing Effect
	if (trailCacheX.size() == TRAIL_SIZE) {
		trailCacheX.pop_back();
		trailCacheY.pop_back();
		trailCacheX.push_front(ball.x);
		trailCacheY.push_front(ball.y);
	}
	else {
		trailCacheX.push_front(ball.x);
		trailCacheY.push_front(ball.y);
		std::cout << trailCacheY.size() << '\n';
	}
	std::list<float>::iterator it1 = trailCacheX.begin();
	std::list<float>::iterator it2 = trailCacheY.begin();
	int ghost = 200;
	for (; it1 != trailCacheX.end() && it2 != trailCacheY.end(); ++it1, ++it2)
	{
		trail.x = *it1;
		trail.y = *it2;
		SDL_SetRenderDrawColor(renderer, 255, 255, 0, ghost);
		SDL_RenderFillRect(renderer, &trail);
		ghost -= 30;
	}
	#pragma endregion

	write("Level " + std::to_string(chapter), 20 + FONT_SIZE / 2 * 6, FONT_SIZE * 1.5);
	write(std::to_string(livesCount), SCREEN_WIDTH / 2 + FONT_SIZE / 2, FONT_SIZE * 1.5);
	write(std::to_string(points), SCREEN_WIDTH - 40 , FONT_SIZE * 1.5);

	//Brick Rendering, Ahpla value decide by healt of the brick
	for (int i = 0; i < coloms * rows; i++) {
		SDL_SetRenderDrawColor(renderer, brickColor.r, brickColor.g, brickColor.b, 255 * bricks[i] / chapter);
		if (i % 2 == 0)SDL_SetRenderDrawColor(renderer, brickColor.g, brickColor.r, brickColor.b, 255 * bricks[i]/chapter);
		if (bricks[i] > 0) {
			setBrick(i);
			SDL_RenderFillRect(renderer, &brick);
		}
	}

	SDL_RenderPresent(renderer);
}

int main(int argc, char* argv[]) {
	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0, &window, &renderer);
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetWindowTitle(window, "Ball The Game");
	TTF_Init();
	font = TTF_OpenFont("font.ttf", FONT_SIZE);

	running = 1;
	static int lastTime = 0;

	color.r = color.g = color.b = 255;
	brickColor.r = 30; brickColor.g = brickColor.b = 100;
	paddle.h = 22; paddle.w = SCREEN_HEIGHT / 4;
	paddle.h = 12;
	ball.w = ball.h = ballSize;
	trail.w = trail.h = ballSize;
	paddle.y = SCREEN_HEIGHT - paddle.h - 32;

	brick.w = (SCREEN_WIDTH - (SPACING * coloms)) / coloms;
	brick.h = 22;
	resetGame(true);

	while (running) {
		lastFrame = SDL_GetTicks();
		if (lastFrame >= (lastTime + 1000)) {
			lastTime = lastFrame;
			fps = frameCount;
			frameCount = 0;
		}
		update();
		input();
		render();
	}

	//Quit The Game
	TTF_CloseFont(font);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}