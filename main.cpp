#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <string>
#include <fstream> 
using namespace sf;

const float TILE_WIDTH = 32;

const float FRICTION = 0.91f;
const float GRAVITY = 2200.0f;
const float TERMINAL_VELOCITY = 500.0f; //maximum vertical velocity




int sign(const int& num) {
    return (num > 0 ? 1 : -1);
}


struct Animation {
    Texture spritesheet;
    IntRect currentSprite;
    float frameWidth = 0;
    int frameCount = 0;
    Clock timer;
    bool flipped = 0;
    float switchTime = 0.12f;
};

struct Tile {
    Sprite sprite;
    Sprite coin;
    bool isSolid = false;
    bool isCoin = false;
    bool isPowerup = false;
};

struct Level {
    int width = 128;
    int height = 32;

    Clock timer;
    int countdown_timer = 120;

    std::string map_string;
    Texture spritesheet;
    Texture feather;
    Texture txCoin;
    Tile* tiles = nullptr; //dynamic array
    Texture backgroundImage;
    Sprite background;

    Text score_text;
    Text timer_text;


    SoundBuffer coinBuffer;
    Sound coinSound;
};

struct Player {
    Sprite body;
    FloatRect hitbox;
    RectangleShape debugger; //red square that shows the hitbox of the player which is smaller than the actual sprite

    Animation anim[3];
    Animation wingsAnimation;
    Sprite wings;
    int animationState = 0; // 0 for idle, 1 for running, 2 for jumping

    int score = 0;
    Vector2f velocity;
    Vector2f acceleration;
    float maxVelocity = 300.0f;
    float minVelocity = 20.0f;
    bool isJumping = false;
    bool canFly = false;
    bool canThrow = false;
};

struct Game {
    RenderWindow window;
    View camera;
    Level map;
    Player player;
    Font font;
};



void initGame(Game& game);
void updateGame(Game& game, float elapsed);
void drawGame(Game& game);

void initPlayer(Player& player);
void movePlayer(Player& player, Level& level, float elapsed);
void updatePlayer(Player& player, Level& level, float elapsed);
void updatePlayerAnimation(Player& player);
void collisionX(Player& player, Level& level);
void collisionY(Player& player, Level& level);
void drawPlayer(Game& game);

void initAnimation(Animation& anim, int width, int height, int frameCount);
void updateAnimation(Animation& anim);


void initLevel(Level& level, Font &font);
void loadLevelFile(Level& level);
void updateLevel(Game& game);
void drawLevel(Game& game);


int main()
{
    Game game;
    initGame(game);

    

    Clock clock;

    while (game.window.isOpen()) {

        updateGame(game, clock.restart().asSeconds());
        game.window.clear(Color::Black);
        drawGame(game);
    }

    return 0;
}



void initGame(Game& game) {
    game.window.create(VideoMode(1536, 864), "Super Hobs");
    game.window.setFramerateLimit(90);

    game.camera.setCenter(Vector2f(350.f, 300.f));
    game.camera.setSize(Vector2f(1056, 594));

    game.font.loadFromFile("./assets/font.ttf");
    game.font.setSmooth(false);

    initPlayer(game.player);
    initLevel(game.map,game.font);
}

void updateGame(Game& game, float elapsed) {

    Event event;
    while (game.window.pollEvent(event))
    {
        switch (event.type) {
        case Event::Closed:
            delete[] game.map.tiles;
            game.map.tiles = nullptr;
            game.window.close(); return; break;

        case Event::KeyPressed:
            if (event.key.scancode == Keyboard::Scan::W && !game.player.isJumping) {
                game.player.velocity.y = -800.0f;
            } break;
        }

    }

    
    updatePlayer(game.player, game.map, elapsed);

    int cameraX = round(game.player.hitbox.left + (3 * TILE_WIDTH));
    int cameraY = round(9 * TILE_WIDTH);

    const int left_boundary = 528, right_boundary = 2000;
    if (cameraX < left_boundary) 
        cameraX = left_boundary;

    if (cameraX > right_boundary)
        cameraX = right_boundary;

    game.camera.setCenter(cameraX, cameraY); // must be integers or else strange lines appear

    updateLevel(game);

}

void drawGame(Game& game) {
    game.window.setView(game.camera);
    if (game.map.tiles != nullptr) {
        drawLevel(game);
    }
    
    drawPlayer(game);
    game.window.display();
}



void initPlayer(Player& player) {

    player.anim[0].spritesheet.loadFromFile("./assets/player/spritesheet_idle.png");
    initAnimation(player.anim[0], 33, 32, 4);

    player.anim[1].spritesheet.loadFromFile("./assets/player/spritesheet_run.png");
    player.anim[1].switchTime = 0.08f;
    initAnimation(player.anim[1], 33, 32, 6);

    player.anim[2].spritesheet.loadFromFile("./assets/player/spritesheet_jump.png");
    player.anim[2].switchTime = 0.5f;
    initAnimation(player.anim[2], 33, 32, 2);


    //wings  
    player.wingsAnimation.spritesheet.loadFromFile("./assets/player/wings.png");
    player.wingsAnimation.switchTime = 0.25f;
    initAnimation(player.wingsAnimation, 16, 16, 2);
    player.wings.setTexture(player.wingsAnimation.spritesheet);
    player.wings.setTextureRect(player.wingsAnimation.currentSprite);
    player.wings.setPosition(player.hitbox.getPosition());
    player.wings.setOrigin(8, 8);
    player.wings.setScale(2, 2);

    player.body.setTexture(player.anim[0].spritesheet);
    player.body.setTextureRect(player.anim[0].currentSprite);
    player.body.setScale(2, 2);

    player.hitbox = FloatRect(player.body.getPosition(), Vector2f(TILE_WIDTH, TILE_WIDTH));

    player.debugger = RectangleShape(Vector2f(TILE_WIDTH, TILE_WIDTH));
    player.debugger.setFillColor(Color::White);

    player.acceleration.x = 50;

}

void updatePlayer(Player& player, Level& level, float elapsed) {
    player.velocity.y += GRAVITY * elapsed;
    player.velocity.x *= FRICTION;
    
    movePlayer(player, level, elapsed);
    updatePlayerAnimation(player);

    player.debugger.setPosition(round(player.hitbox.left),round(player.hitbox.top-250));
    player.body.setTexture(player.anim[player.animationState].spritesheet);
    player.body.setTextureRect(player.anim[player.animationState].currentSprite);
    player.wings.setTextureRect(player.wingsAnimation.currentSprite);


}

void collisionX(Player& player, Level& level) {
    
    if (player.hitbox.left < 0) {
        player.velocity.x = 0;
        player.hitbox.left = 0;
    }
        
    
    int left_tile = player.hitbox.getPosition().x / TILE_WIDTH;
    int right_tile = left_tile + 1;
    int vertical_pos = player.hitbox.getPosition().y / TILE_WIDTH;

    int moveDirection = sign(player.velocity.x);

    for (int i = vertical_pos; i <= vertical_pos; i++) {
        for (int j = left_tile; j <= right_tile; j++) {
            Tile &currentTile = level.tiles[j + i * level.width];

            if (currentTile.isSolid && player.hitbox.intersects(currentTile.sprite.getGlobalBounds())) {

                player.velocity.x = 0;
                player.hitbox.left = currentTile.sprite.getPosition().x - TILE_WIDTH * moveDirection;

            }
            
            if (currentTile.isPowerup && player.hitbox.intersects(currentTile.sprite.getGlobalBounds())) {
                player.canFly = true;
                currentTile.isPowerup = false;
            }

            if (currentTile.isCoin && player.hitbox.intersects(currentTile.coin.getGlobalBounds())) {
                currentTile.isCoin = false;
                player.score++;
                level.coinBuffer.loadFromFile("./assets/audio/coin.wav");
                level.coinSound.setBuffer(level.coinBuffer);
                level.coinSound.play();
            }

        }
    }
}

void collisionY(Player& player, Level& level) { // no head collisions yet

    int above_tile = player.hitbox.top / TILE_WIDTH;
    int below_tile = above_tile + 1;
    float horizontal_pos = player.hitbox.left / TILE_WIDTH;

    if (player.hitbox.top < 0)
        player.hitbox.top = 0;

    for (int j = horizontal_pos; j <= horizontal_pos + 1; j++) {
        Tile &currentTile = level.tiles[j + below_tile * level.width];

        if (currentTile.isSolid && player.velocity.y >= 0 && player.hitbox.intersects(currentTile.sprite.getGlobalBounds())) {

            player.isJumping = false;
            player.velocity.y = 0;
            player.hitbox.top = currentTile.sprite.getPosition().y - TILE_WIDTH;

        }
    }
}

void drawPlayer(Game& game) {
    if (game.player.canFly)
        game.window.draw(game.player.wings);
    
    game.window.draw(game.player.body);
    //game.window.draw(game.player.debugger);
}

void movePlayer(Player& player, Level& level, float elapsed) {

    if (Keyboard::isKeyPressed(Keyboard::Scan::D)) {
        for (int i = 0; i < 3; i++) {
            player.anim[i].flipped = 0;
        }
            player.wingsAnimation.flipped = 0;

        player.velocity.x += player.acceleration.x;
    }

    if (Keyboard::isKeyPressed(Keyboard::Scan::A)) {
        for (int i = 0; i < 3; i++){
            player.anim[i].flipped = 1;
        }
            player.wingsAnimation.flipped = 1;


        player.velocity.x += player.acceleration.x * -1;
    }

    if (Keyboard::isKeyPressed(Keyboard::Scan::Space) && player.canFly) {
        player.velocity.y = -400;
    }


    // friction is a percentage, so velocity never really reaches zero, this sets the velocity the velocity to zero once it reaches a small amount 
    if (abs(player.velocity.x) < player.minVelocity)
        player.velocity.x = 0;


    if (abs(player.velocity.x) > player.maxVelocity)
        player.velocity.x = player.maxVelocity * sign(player.velocity.x);

    if (player.velocity.y > TERMINAL_VELOCITY)
        player.velocity.y = TERMINAL_VELOCITY;

    player.hitbox.left += player.velocity.x * elapsed;
    player.hitbox.top += player.velocity.y * elapsed;
    collisionX(player, level);
    collisionY(player, level);
    player.body.setPosition(player.hitbox.left - 16, player.hitbox.top - 32);
    player.wings.setPosition(player.hitbox.left + (player.wingsAnimation.flipped * 32), player.hitbox.top);
}

void updatePlayerAnimation(Player& player) {

    if (player.velocity.x == 0 && player.velocity.y == 0)
        player.animationState = 0;

    if (abs(player.velocity.x) > 0 && player.velocity.y == 0)
        player.animationState = 1;


    updateAnimation(player.wingsAnimation);
    updateAnimation(player.anim[player.animationState]);

    if (player.velocity.y < 0) {
        player.anim[2].currentSprite.left = 0 + (player.anim[2].flipped * 32);
        player.animationState = 2;
        player.isJumping = true;
    }


    if (player.velocity.y > 0) {
        player.anim[2].currentSprite.left = 32 + (player.anim[2].flipped * 32);
        player.animationState = 2;
    }


}


void initAnimation(Animation& anim, int width, int height, int frameCount) {
    anim.frameCount = frameCount;
    anim.currentSprite = IntRect(0, 0, width, height);
    anim.frameWidth = anim.currentSprite.width;
}

void updateAnimation(Animation& anim) {

    if (anim.flipped) //setting width to negative flips image
        anim.currentSprite.width = -anim.frameWidth;
    else
        anim.currentSprite.width = anim.frameWidth;


    if (anim.timer.getElapsedTime().asSeconds() > anim.switchTime) {

        if (anim.currentSprite.left >= (anim.frameCount - 1 + anim.flipped) * abs(anim.currentSprite.width))
            anim.currentSprite.left = anim.flipped * abs(anim.currentSprite.width);
        else
            anim.currentSprite.left += abs(anim.currentSprite.width);

        anim.timer.restart();

    }
}



void initLevel(Level& level,Font &font) {
    level.spritesheet.loadFromFile("./assets/level/level_tileset.png");
    level.feather.loadFromFile("./assets/level/feather.png");
    level.backgroundImage.loadFromFile("./assets/level/back.png");
    level.txCoin.loadFromFile("./assets/level/gem.png");
    loadLevelFile(level);

    level.background.setTexture(level.backgroundImage);
    level.background.setColor(Color(230, 230, 230, 255));
    level.background.setScale(3, 3);
    level.background.setOrigin(192, 120); // set origin to center instead of top left

    level.score_text.setFont(font);
    level.score_text.setCharacterSize(48);
    level.score_text.setFillColor(Color::White);

    level.timer_text.setFont(font);
    level.timer_text.setPosition(0, 0);
    level.timer_text.setCharacterSize(48);

    level.tiles = new Tile[level.height * level.width];

    for (int i = 0; i < level.height; i++) {
        for (int j = 0; j < level.width; j++) {

            level.tiles[j + i * level.width].sprite.setPosition(j * TILE_WIDTH, i * TILE_WIDTH);
            level.tiles[j + i * level.width].sprite.setTexture(level.spritesheet);
            level.tiles[j + i * level.width].sprite.setScale(2, 2);

            level.tiles[j + i * level.width].coin.setPosition(j * TILE_WIDTH, i * TILE_WIDTH);
            level.tiles[j + i * level.width].coin.setTexture(level.txCoin);

            switch (level.map_string.at(j + i * level.width)) {
            case '1':
                level.tiles[j + i * level.width].sprite.setTextureRect(IntRect(16, 16, 16, 16));
                level.tiles[j + i * level.width].isSolid = true;
                break;
            case '2':
                level.tiles[j + i * level.width].sprite.setTextureRect(IntRect(16 + 32, 16, 16, 16));
                level.tiles[j + i * level.width].isSolid = true;
                break;
            case '3':
                level.tiles[j + i * level.width].sprite.setTextureRect(IntRect(16 + 64, 16, 16, 16));
                level.tiles[j + i * level.width].isSolid = true;
                break;
            case '4':
                level.tiles[j + i * level.width].sprite.setTextureRect(IntRect(16, 16 + 32, 16, 16));
                level.tiles[j + i * level.width].isSolid = true;
                break;
            case '5':
                level.tiles[j + i * level.width].sprite.setTextureRect(IntRect(16 + 32, 16 + 32, 16, 16));
                level.tiles[j + i * level.width].isSolid = true;
                break;
            case '6':
                level.tiles[j + i * level.width].sprite.setTextureRect(IntRect(16 + 64, 16 + 32, 16, 16));
                level.tiles[j + i * level.width].isSolid = true;
                break;
            case '7':
                level.tiles[j + i * level.width].sprite.setTextureRect(IntRect(16, 16 + 64, 16, 16));
                level.tiles[j + i * level.width].isSolid = true;
                break;
            case '8':
                level.tiles[j + i * level.width].sprite.setTextureRect(IntRect(16 + 32, 16 + 64, 16, 16));
                level.tiles[j + i * level.width].isSolid = true;
                break;
            case '9':
                level.tiles[j + i * level.width].sprite.setTextureRect(IntRect(16 + 64, 16 + 64, 16, 16));
                level.tiles[j + i * level.width].isSolid = true;
                break;
            case 'F':
                level.tiles[j + i * level.width].sprite.setTexture(level.feather);
                level.tiles[j + i * level.width].sprite.setTextureRect(IntRect(0,0, 16, 16));
                level.tiles[j + i * level.width].isPowerup = true;
                break;
            case 'C':
                level.tiles[j + i * level.width].coin.setTextureRect(IntRect(0, 0, 16, 16));
                level.tiles[j + i * level.width].coin.setScale(2, 2);
                level.tiles[j + i * level.width].isCoin = true;
                break; 
            default: break;
            }


        }
    }

}

void loadLevelFile(Level& level) {
    std::ifstream levelFile("./assets/level/level1.txt", std::ios::in);
    std::string temp;
    
    for (int i = 0; i < level.height && !levelFile.eof(); i++) {
        levelFile >> temp;
        level.map_string += temp;
    }
    levelFile.close();
}

void updateLevel(Game& game) {
    game.map.score_text.setPosition(round(game.camera.getCenter().x-100), 0);
    game.map.score_text.setString("SCORE: " + std::to_string(game.player.score));

    if (game.map.timer.getElapsedTime().asSeconds() >= 1) {
        game.map.countdown_timer--;
        game.map.timer_text.setString("TIME: " + std::to_string(game.map.countdown_timer));
        game.map.timer.restart();
        if (game.map.countdown_timer <= 0)
            game.window.close();
    }

    game.map.timer_text.setPosition(round(game.camera.getCenter().x + 200), 0);


    game.map.background.setPosition(game.camera.getCenter());


}

void drawLevel(Game& game) {

    game.window.draw(game.map.background);
    game.window.draw(game.map.timer_text);
    game.window.draw(game.map.score_text);

    for (int i = 0; i < game.map.height; i++) {
        for (int j = 0; j < game.map.width; j++) {

            Tile &currentTile = game.map.tiles[j + i * game.map.width];
            if (currentTile.isSolid || currentTile.isPowerup) { //make sure sprite isn't empty
                game.window.draw(game.map.tiles[j + i * game.map.width].sprite);
            }

            if (currentTile.isCoin) {
                game.window.draw(game.map.tiles[j + i * game.map.width].coin);
            }
                

        }
    }
}