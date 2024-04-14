#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>
#include <fstream> 
using namespace sf;

const float TILE_WIDTH = 32;

const float FRICTION = 0.91f;
const float GRAVITY = 25.0f;
const float TERMINAL_VELOCITY = 500.0f; //maximum vertical velocity

int sign(const int& num) {
    return (num > 0 ? 1 : -1);
}


struct Animation {
    Texture texture;
    IntRect currentSprite;
    float frame_width = 0;
    int frameCount = 6;
    Clock animationTimer;
    bool flipped = 0;
    float switchTime = 0.12f;
};

struct Tile {
    Sprite sprite;
    bool isSolid = false;
    bool isCoin = false;
};

struct Level {
    int width = 128;
    int height = 32;
    Clock timer;
    std::string map_string;
    Texture map_texture;
    Tile* tiles;
    Texture background_texture;
    Sprite background;
};

struct Player {
    Sprite body;
    FloatRect hitbox;
    RectangleShape debugger; //red square that shows the hitbox of the player which is smaller than the actual sprite
    Animation anim[3];
    int animationState = 0; // 0 for idle, 1 for running, 2 for jumping
    

    Vector2f velocity;
    Vector2f acceleration;
    float max_velocity = 300.0f;
    float min_velocity = 20.0f;
    bool isJumping = false;
};

struct Game {
    RenderWindow window;
    View camera; // this is the camera
    Level map;
    Player player;
};


void initGame(Game& game);
void updateGame(Game& game, const float& elapsed);
void drawGame(Game& game);

void initLevel(Level& level);
void loadLevelFile(Level& level);
void updateLevel(Game& game);
void drawLevel(Game& game);

void initAnimation(Animation& anim, int width, int height, int frameCount);
void updateAnimation(Animation& anim);

void initPlayer(Player& player);
void movePlayer(Player& player, Level& level, const float& elapsed);
void updatePlayer(Player& player, Level& level, const float& elapsed);
void updatePlayerAnimation(Player& player);
void collisionX(Player& player, Level& level);
void collisionY(Player& player, Level& level);




int main()
{
    Game game;
    initGame(game);
    
    Clock clock;
    

    while (game.window.isOpen()) {
        updateGame(game,clock.restart().asSeconds());
        game.window.clear();
        drawGame(game);
    }

    return 0;
}




void initGame(Game& game) {
    game.window.create(VideoMode(1536, 864), "Super Hobs");
    game.window.setFramerateLimit(120);
    game.camera.setCenter(Vector2f(350.f, 300.f));
    game.camera.setSize(Vector2f(1056, 594));

    initPlayer(game.player);
    initLevel(game.map);
}

void updateGame(Game& game, const float& elapsed) {

    //timer logic here

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

    if (Keyboard::isKeyPressed(Keyboard::Scan::D)) {
        for (int i = 0; i < 3; i++)
            game.player.anim[i].flipped = 0;

        game.player.velocity.x += game.player.acceleration.x;

    }

    if (Keyboard::isKeyPressed(Keyboard::Scan::A)) {
        for (int i = 0; i < 3; i++)
            game.player.anim[i].flipped = 1;

        game.player.velocity.x += game.player.acceleration.x * -1;
    }
    
    updateLevel(game);
    updatePlayer(game.player, game.map, elapsed);
    game.camera.setCenter(round(game.player.hitbox.left + (3 * TILE_WIDTH)), round(9 * TILE_WIDTH)); // must be integers or else strange lines appear

}

void drawGame(Game& game) {
    game.window.setView(game.camera);
    if (game.map.tiles != nullptr) {
        drawLevel(game);
    }
    //game.window.draw(game.player.debugger);
    game.window.draw(game.player.body);
    game.window.display();
}



void initPlayer(Player& player) {

    player.anim[0].texture.loadFromFile("./assets/player/spritesheet_idle.png");
    initAnimation(player.anim[0], 33, 32, 4);

    player.anim[1].texture.loadFromFile("./assets/player/spritesheet_run.png");
    player.anim[1].switchTime = 0.08f;
    initAnimation(player.anim[1], 33, 32, 6);

    player.anim[2].texture.loadFromFile("./assets/player/spritesheet_jump.png");
    player.anim[2].switchTime = 0.5f;
    initAnimation(player.anim[2], 33, 32, 2);


    player.body.setTexture(player.anim[0].texture);
    player.body.setTextureRect(player.anim[0].currentSprite);
    player.body.setScale(2, 2);
    player.hitbox = FloatRect(player.body.getPosition(), Vector2f(TILE_WIDTH, TILE_WIDTH));
    player.debugger = RectangleShape(Vector2f(TILE_WIDTH, TILE_WIDTH));
    player.debugger.setFillColor(Color::Red);
   

    player.acceleration.x = 50;
}

void updatePlayer(Player& player, Level& level, const float& elapsed) {
    player.velocity.y += GRAVITY;
    player.velocity.x *= FRICTION;
    
    movePlayer(player, level, elapsed);
    updatePlayerAnimation(player); //still have to fix
    
    //player.debugger.setPosition(player.hitbox.left,player.hitbox.top);
    player.body.setTexture(player.anim[player.animationState].texture);
    player.body.setTextureRect(player.anim[player.animationState].currentSprite);

}

void collisionX(Player& player, Level& level) {
    int left_tile = player.hitbox.getPosition().x / TILE_WIDTH;
    int right_tile = left_tile + 1;
    int vertical_pos = player.hitbox.getPosition().y / TILE_WIDTH;

    int moveDirection = sign(player.velocity.x);

    for (int i = vertical_pos; i <= vertical_pos; i++) {
        for (int j = left_tile; j <= right_tile; j++) {
            Tile currentTile = level.tiles[j + i * level.width];

            if (currentTile.isSolid && player.hitbox.intersects(currentTile.sprite.getGlobalBounds())) {

                player.velocity.x = 0;
                player.hitbox.left = currentTile.sprite.getPosition().x - TILE_WIDTH * moveDirection;

            }

            //TODO: Coin (gem) logic here, condition should be the same as above except instead of checking for isSolid, check for isCoin
        }
    }

}

void collisionY(Player& player, Level& level) { // no head collisions yet
    int above_tile = player.hitbox.top / TILE_WIDTH;
    int below_tile = above_tile + 1;
    float horizontal_pos = player.hitbox.left / TILE_WIDTH;

    for (int j = horizontal_pos; j <= horizontal_pos+1; j++) {
        Tile currentTile = level.tiles[j + below_tile * level.width];

        if (currentTile.isSolid && player.velocity.y >= 0 && player.hitbox.intersects(currentTile.sprite.getGlobalBounds()) ) {

            player.isJumping = false;
            player.velocity.y = 0;
            player.hitbox.top = currentTile.sprite.getPosition().y - TILE_WIDTH;

        }

    }

}

void movePlayer(Player& player, Level& level, const float& elapsed) {
    // friction is a percentage, so velocity never really reaches zero, this sets the velocity the velocity to zero once it reaches a small amount 
    if (abs(player.velocity.x) < player.min_velocity )
        player.velocity.x = 0;

   
    if (abs(player.velocity.x) > player.max_velocity)
        player.velocity.x = player.max_velocity * sign(player.velocity.x);

    if (player.velocity.y > TERMINAL_VELOCITY)
        player.velocity.y = TERMINAL_VELOCITY;

    player.hitbox.left += player.velocity.x * elapsed;
    player.hitbox.top += player.velocity.y * elapsed;
    collisionX(player, level);
    collisionY(player, level);
    player.body.setPosition(player.hitbox.left - 16, player.hitbox.top - 32);
}

void updatePlayerAnimation(Player& player) {
    if (player.velocity.x == 0)
        player.animationState = 0;

    if (abs(player.velocity.x) > 0)
        player.animationState = 1;

    if (player.velocity.y < 0) {
        player.anim[2].currentSprite.left = 0 + (player.anim[2].flipped * 32);
        player.animationState = 2;
        player.isJumping = true;
    }

    if (player.velocity.y > 0) {
        player.anim[2].currentSprite.left = 32 + (player.anim[2].flipped * 32);
        player.animationState = 2;
    }

    updateAnimation(player.anim[player.animationState]);
}

void initAnimation(Animation& anim, int width, int height,int frameCount) {
    anim.frameCount = frameCount;
    anim.currentSprite = IntRect(0, 0, width, height);
    anim.frame_width = anim.currentSprite.width;
}

void updateAnimation(Animation& anim) {
    if (anim.animationTimer.getElapsedTime().asSeconds() > anim.switchTime) {

        if (anim.flipped) //setting width to negative flips image
            anim.currentSprite.width = -anim.frame_width;
        else 
            anim.currentSprite.width = anim.frame_width;


        if (anim.currentSprite.left >= (anim.frameCount-1 + anim.flipped ) * abs(anim.currentSprite.width) ) 
            anim.currentSprite.left = 0 + anim.flipped * abs(anim.currentSprite.width);
        else
            anim.currentSprite.left += abs(anim.currentSprite.width);
        

        anim.animationTimer.restart();
    }
}


void initLevel(Level& level) {
    level.map_texture.loadFromFile("./assets/level/level_tileset.png");
    level.background_texture.loadFromFile("./assets/level/back.png");
    loadLevelFile(level);
    
    level.background.setTexture(level.background_texture);
    level.background.setColor(Color(230, 230, 230, 255));
    level.background.setScale(3, 3);
    level.background.setOrigin(192, 120);
    level.tiles = new Tile[level.height * level.width];

    for (int i = 0; i < level.height; i++) {
        for (int j = 0; j < level.width; j++) {

            level.tiles[j + i * level.width].sprite.setPosition(j * TILE_WIDTH, i * TILE_WIDTH);
            level.tiles[j + i * level.width].sprite.setTexture(level.map_texture);
            level.tiles[j + i * level.width].sprite.setScale(2, 2);

            switch (level.map_string.at(j + i * level.width)) {
                case '1':
                    level.tiles[j + i * level.width].sprite.setTextureRect(IntRect(16, 16, 16, 16));
                    level.tiles[j + i * level.width].isSolid = true;
					break;
                case '2':
                    level.tiles[j + i * level.width].sprite.setTextureRect(IntRect(16+32, 16, 16, 16));
                    level.tiles[j + i * level.width].isSolid = true;
                    break;
                case '3':
                    level.tiles[j + i * level.width].sprite.setTextureRect(IntRect(16+64, 16, 16, 16));
                    level.tiles[j + i * level.width].isSolid = true;
                    break;
                case '4':
                    level.tiles[j + i * level.width].sprite.setTextureRect(IntRect(16, 16+32, 16, 16));
                    level.tiles[j + i * level.width].isSolid = true;
                    break;
                case '5':
                    level.tiles[j + i * level.width].sprite.setTextureRect(IntRect(16+32, 16 + 32, 16, 16));
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
                case 'C': break; // coin here, texture is "./assets/level/gem.png"
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
    game.map.background.setPosition(game.camera.getCenter());
}

void drawLevel(Game& game) {

        game.window.draw(game.map.background);
        for (int i = 0; i < game.map.height; i++) {
            for (int j = 0; j < game.map.width; j++) {

                Tile currentTile = game.map.tiles[j + i * game.map.width];
                if (currentTile.isSolid || currentTile.isCoin) { //make sure sprite isn't empty
                    game.window.draw(game.map.tiles[j + i * game.map.width].sprite);
                }

            }
        }
}