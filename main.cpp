#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>
#include <fstream>

const float TILE_WIDTH = 32;

const float FRICTION = 0.91f;
const float GRAVITY = 25.0f;
const float TERMINAL_VELOCITY = 500.0f;

int sign(int num) {
    return (num > 0 ? 1 : -1);
}


struct Animation {
    sf::Texture texture;
    sf::IntRect currentSprite;
    float frame_width = 0;
    int frameCount = 6;
    sf::Clock animationTimer;
    bool flipped = 0;
    float switchTime = 0.12f;
};

struct Tile {
    sf::Sprite sprite;
    bool solid = 0;
};

struct Level {
    int width = 128;
    int height = 32;
    std::string map_string;
    Tile* tiles;
    sf::Texture map_texture;
    sf::Texture background_texture;
    sf::Sprite background;
};

struct Player {
    sf::Sprite body;
    sf::FloatRect hitbox;
    sf::RectangleShape debugger; 
    Animation anim[3];
    int animationState = 0; // 0 for idle, 1 for running, 2 for jumping
    

    sf::Vector2f velocity;
    sf::Vector2f acceleration;
    float max_velocity = 300.0f;
    bool isJumping = false;
};

struct Game {
    sf::RenderWindow window;
    sf::View view;
    Level map;
    Player player;
};


void initGame(Game& game);
void updateGame(Game& game,float elapsed);
void drawGame(Game& game);

void initLevel(Level& level);
void loadLevelFile(Level& level);
void updateLevel(Game& game);
void drawLevel(Game& game);

void initAnimation(Animation& anim, int width, int height, int frameCount);
void updateAnimation(Animation& anim);

void initPlayer(Player& player);
void movePlayer(Player& player,const sf::Vector2i& direction,const float& elapsed);
void updatePlayer(Player& player, Level& level, float elapsed);
bool collided_X(Player& player, Level& level);
bool collided_Y(Player& player, Level& level);



int main()
{
    Game game;
    initGame(game);
    
    sf::Clock clock;

    while (game.window.isOpen()) {
        updateGame(game,clock.restart().asSeconds());
        game.window.clear();
        drawGame(game);
    }

    return 0;
}




void initGame(Game& game) {
    game.window.create(sf::VideoMode(1536, 864), "test");
    game.window.setFramerateLimit(120);
    game.view.setCenter(sf::Vector2f(350.f, 300.f));
    game.view.setSize(sf::Vector2f(1056, 594));

    initPlayer(game.player);
    initLevel(game.map);
}

void updateGame(Game& game, float elapsed) {

    
    sf::Event event;
    while (game.window.pollEvent(event))
    {
        switch (event.type) {
            case sf::Event::Closed:
                delete[] game.map.tiles;
                game.map.tiles = nullptr;
                game.window.close(); return; break;

            case sf::Event::KeyPressed:
                if (event.key.scancode == sf::Keyboard::Scan::W && !game.player.isJumping) {
                    game.player.velocity.y = -800.0f;
                } break;
        }

    }

    

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::D)) {
        for (int i = 0; i < 3; i++)
            game.player.anim[i].flipped = 0;

        game.player.velocity.x += game.player.acceleration.x;

    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::A)) {
        for (int i = 0; i < 3; i++)
            game.player.anim[i].flipped = 1;

        game.player.velocity.x += game.player.acceleration.x * -1;
    }
    

    

    updatePlayer(game.player,game.map, elapsed);
    game.view.setCenter(round(game.player.hitbox.left + (3 * TILE_WIDTH)), round(9 * TILE_WIDTH));
    updateLevel(game);
}

void drawGame(Game& game) {
    game.window.setView(game.view);
    if (game.map.tiles != nullptr) {
        drawLevel(game);
    }
    game.window.draw(game.player.debugger);
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
    player.hitbox = sf::FloatRect(player.body.getPosition(), sf::Vector2f(TILE_WIDTH, TILE_WIDTH));
    player.debugger = sf::RectangleShape(sf::Vector2f(TILE_WIDTH, TILE_WIDTH));
    player.debugger.setFillColor(sf::Color::Red);
    

    player.velocity.x = 0;
    player.acceleration.x = 50;
}

void updatePlayer(Player& player, Level& level, float elapsed) {


    player.velocity.y += GRAVITY;
    player.velocity.x *= FRICTION;


    if (abs(player.velocity.x) <= 20.0f)
        player.velocity.x = 0;

    if (abs(player.velocity.x) >= player.max_velocity)
        player.velocity.x = player.max_velocity * sign(player.velocity.x);

    if (player.velocity.y >= TERMINAL_VELOCITY)
        player.velocity.y = TERMINAL_VELOCITY;

    
    player.hitbox.left += player.velocity.x * elapsed;
    player.hitbox.top += player.velocity.y * elapsed;
    collided_X(player, level);
    collided_Y(player, level);
    player.body.setPosition(player.hitbox.left - 16, player.hitbox.top - 32);

    // animation logic
    if (player.velocity.x == 0) 
        player.animationState = 0;
    

    if (abs(player.velocity.x) > 0)
        player.animationState = 1;

    if (player.velocity.y < 0) {
        player.anim[2].currentSprite.left = 0 + (player.anim[2].flipped * 32);
        player.animationState = 2;
        player.isJumping = true;
    }

    if (player.velocity.y > 0 ) {
        player.anim[2].currentSprite.left = 32 + (player.anim[2].flipped * 32);
        player.animationState = 2;
    }

    //collision
    



    std::cout << player.velocity.y << '\n';

    
    player.debugger.setPosition(player.hitbox.left,player.hitbox.top);
    updateAnimation(player.anim[player.animationState]);
    player.body.setTexture(player.anim[player.animationState].texture);
    player.body.setTextureRect(player.anim[player.animationState].currentSprite);

}

bool collided_X(Player& player, Level& level) {
    int left_tile = player.hitbox.getPosition().x / TILE_WIDTH;
    int right_tile = left_tile + 1;
    int vertical_pos = player.hitbox.getPosition().y / TILE_WIDTH;

    int moveDirection = sign(player.velocity.x);

    for (int i = vertical_pos; i <= vertical_pos; i++) {
        for (int j = left_tile; j <= right_tile; j++) {
            Tile currentTile = level.tiles[j + i * level.width];

            if (currentTile.solid && player.hitbox.intersects(currentTile.sprite.getGlobalBounds())) {

                player.velocity.x = 0;
                player.hitbox.left = currentTile.sprite.getPosition().x - TILE_WIDTH * moveDirection;
                return true;

            }
        }
    }

    return false;
}

bool collided_Y(Player& player, Level& level) {
    int above_tile = player.hitbox.top / TILE_WIDTH;
    int below_tile = above_tile + 1;
    float horizontal_pos = player.hitbox.left / TILE_WIDTH;

    for (int j = horizontal_pos; j <= floor(horizontal_pos+1.0f); j++) {
        Tile currentTile = level.tiles[j + below_tile * level.width];

        if (currentTile.solid && player.velocity.y >= 0 && player.hitbox.intersects(currentTile.sprite.getGlobalBounds()) ) {

            player.isJumping = false;
            player.velocity.y = 0;
            player.hitbox.top = currentTile.sprite.getPosition().y - TILE_WIDTH;
            return true;

        }

    }

    return false;
}



void initAnimation(Animation& anim, int width, int height,int frameCount) {
    anim.frameCount = frameCount;
    anim.currentSprite = sf::IntRect(0, 0, width, height);
    anim.frame_width = anim.currentSprite.width;
}

void updateAnimation(Animation& anim) {


    if (anim.animationTimer.getElapsedTime().asSeconds() > anim.switchTime) {

        if (anim.flipped) //setting width to negative mirrors image
            anim.currentSprite.width = -anim.frame_width;
        else 
            anim.currentSprite.width = anim.frame_width;


        if (anim.currentSprite.left >= (anim.frameCount-1 + anim.flipped ) * std::abs(anim.currentSprite.width) ) 
            anim.currentSprite.left = 0 + anim.flipped * std::abs(anim.currentSprite.width);
        else
            anim.currentSprite.left += std::abs(anim.currentSprite.width);
        

        //std::cout << (anim.currentSprite.left ) << std::endl;
        anim.animationTimer.restart();
    }
}


void initLevel(Level& level) {
    level.map_texture.loadFromFile("./assets/level/level_tileset.png");
    level.background_texture.loadFromFile("./assets/level/back.png");
    loadLevelFile(level);
    
    level.background.setTexture(level.background_texture);
    level.background.setColor(sf::Color(230, 230, 230, 255));
    level.background.setScale(3, 3);
    level.background.setOrigin(192, 120);
    level.tiles = new Tile[level.height * level.width];

    for (int i = 0; i < level.height; i++) {
        for (int j = 0; j < level.width; j++) {

            level.tiles[j + i * level.width].sprite.setTexture(level.map_texture);
            level.tiles[j + i * level.width].sprite.setPosition(j * TILE_WIDTH, i * TILE_WIDTH);
            level.tiles[j + i * level.width].sprite.setScale(2, 2);

            switch (level.map_string.at(j + i * level.width)) {
                case '1':
                    level.tiles[j + i * level.width].sprite.setTextureRect(sf::IntRect(16, 16, 16, 16));
                    level.tiles[j + i * level.width].solid = true;
					break;
                case '2':
                    level.tiles[j + i * level.width].sprite.setTextureRect(sf::IntRect(16+32, 16, 16, 16));
                    level.tiles[j + i * level.width].solid = true;
                    break;
                case '3':
                    level.tiles[j + i * level.width].sprite.setTextureRect(sf::IntRect(16+64, 16, 16, 16));
                    level.tiles[j + i * level.width].solid = true;
                    break;
                case '4':
                    level.tiles[j + i * level.width].sprite.setTextureRect(sf::IntRect(16, 16+32, 16, 16));
                    level.tiles[j + i * level.width].solid = true;
                    break;
                case '5':
                    level.tiles[j + i * level.width].sprite.setTextureRect(sf::IntRect(16+32, 16 + 32, 16, 16));
                    level.tiles[j + i * level.width].solid = true;
                    break;
                case '6':
                    level.tiles[j + i * level.width].sprite.setTextureRect(sf::IntRect(16 + 64, 16 + 32, 16, 16));
                    level.tiles[j + i * level.width].solid = true;
                    break;
                case '7': 
                    level.tiles[j + i * level.width].sprite.setTextureRect(sf::IntRect(16, 16 + 64, 16, 16));
                    level.tiles[j + i * level.width].solid = true;
                    break;
                case '8':
                    level.tiles[j + i * level.width].sprite.setTextureRect(sf::IntRect(16 + 32, 16 + 64, 16, 16));
                    level.tiles[j + i * level.width].solid = true;
                    break;
                case '9':
                    level.tiles[j + i * level.width].sprite.setTextureRect(sf::IntRect(16 + 64, 16 + 64, 16, 16));
                    level.tiles[j + i * level.width].solid = true;
                    break;
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
    game.map.background.setPosition(game.view.getCenter());
}

void drawLevel(Game& game) {

        game.window.draw(game.map.background);
        for (int i = 0; i < game.map.height; i++) {
            for (int j = 0; j < game.map.width; j++) {

                if (game.map.map_string.at(j + i * game.map.width) != '.') { //make sure sprite isn't empty
                    game.window.draw(game.map.tiles[j + i * game.map.width].sprite);
                }

            }
        }
}