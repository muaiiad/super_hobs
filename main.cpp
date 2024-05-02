#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <string>
#include <fstream> 
using namespace sf;

const float TILE_WIDTH = 32;

bool move_player = true;
bool Win = false;
bool gameover = 0;

const float FRICTION = 0.91f;
const float GRAVITY = 2200.0f;
const float TERMINAL_VELOCITY = 500.0f; //maximum vertical velocity


int sign(const int& num) {
    return (num > 0 ? 1 : -1);
}
int pagenum = 10;
struct Menu {
    Text mainmenu[3];
    Font font;
    int selected = 0;
};

void makeMenu(Menu& menu, float width, float height);
void updateMenu(Menu& menu, RenderWindow& window);
void drawMenu(Menu& menu, RenderWindow& window);
void movedown(Menu& menu);
void moveup(Menu& menu);


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
    Sprite spike;
    Sprite flag;
    bool isSolid = false;
    bool isCoin = false;
    bool isSpike = false;
    bool isPowerup = false;
    bool isPipe = false;
    bool isFlag = false;
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
    Texture pipe;
    Texture  spikeTexture;
    Texture flagTexture;

    Tile* tiles = nullptr; //dynamic array
    Texture backgroundImage;
    Sprite background;

    Text score_text;
    Text timer_text;
    Text win_text;


    SoundBuffer coinBuffer;
    Sound coinSound;
};

struct Player {
    Sprite body;
    FloatRect hitbox;
    FloatRect headPoint;
    RectangleShape debugger; //red square that shows the hitbox of the player which is smaller than the actual sprite

    Animation anim[4];
    Animation wingsAnimation;
    Sprite wings;
    int animationState = 0; // 0 for idle, 1 for running, 2 for jumping

    int score = 0;
    int health = 3;

    Vector2i velocity;
    Vector2i acceleration;
    float maxVelocity = 300.0f;
    float minVelocity = 20.0f;
    bool isJumping = false;
    bool canFly = false;
    bool canThrow = false;
};

struct Enemy {
    Sprite body;
    Animation anim[1];
    Vector2f velocity;
    int health;
    int animationState = 0;
    bool isalive = true;
};

struct Bear {
    Sprite body;
    Animation anim[1];
    Vector2f velocity;
    int health;
    int animationState = 0;
    bool isalive = true;
};


struct Game {
    RenderWindow window;
    View camera;
    Level map;
    Player player;
    Font font;

    bool pause = 0;
    Text pausetext;
    Text gameovertext;
    RectangleShape blur;

    Enemy enemy;
    Bear bear;
};



void initGame(Game& game);
void updateGame(Game& game, float elapsed);
void drawGame(Game& game);

void initPlayer(Player& player);
void movePlayer(Player& player, Level& level, float elapsed);
void updatePlayer(Player& player, Level& level, float elapsed);
void updatePlayerAnimation(Player& player);
void collisionX(Player& player, Level& level);
bool collisionY(Player& player, Level& level);
bool collisionHead(Player& player, Level& level);
void drawPlayer(Game& game);

void initEnemy(Enemy& enemy);
void moveEnemy(Enemy& enemy, Level level, float elapsed);
void updateEnemy(Enemy& enemy, Level& level, float elapsed);
void updateEnemyAnimation(Enemy& player);
void collisionX(Enemy& enemy, Level& level);
void collisionY(Enemy& enemy, Level& level);
void drawEnemy(Game& game);
void CollisionPlayerWithEnemy(Enemy& enemy, Player& player);
void CollisionPlayerWithEnemy(Bear& bear, Player& player);

void initbear(Bear& bear);
void movebear(Bear& bear, Level level, float elapsed);
void updateBear(Bear& bear, Level& level, float elapsed);
void updatebearAnimation(Bear& bear);
void drawBear(Game& game);
void collisionX(Bear& bear, Level& level);
void collisionY(Bear& bear, Level& level);

void initAnimation(Animation& anim, int width, int height, int frameCount);
void updateAnimation(Animation& anim);


void initLevel(Level& level, Font &font);
void loadLevelFile(Level& level);
void updateLevel(Game& game);
void drawLevel(Game& game);


int main()
{
    std::string name;
    Menu menu;
    Game game;
    initGame(game);
    makeMenu(menu, 1536, 864);
    

    Clock clock;

    while (game.window.isOpen()) {
        switch (pagenum) {
        case 0:
            updateGame(game, clock.restart().asSeconds());
            game.window.clear(Color::Black);
            drawGame(game);
            break;
        case 1: break;
        case 2: game.window.close();
            break;
        case 3:
            break;
        case 10:
            updateMenu(menu, game.window);
            drawMenu(menu, game.window);
        }
        game.window.display();
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

    game.blur.setPosition(Vector2f(0, 0));
    game.blur.setScale(Vector2f(3, 3));
    game.blur.setSize(Vector2f(1536, 875));
    game.blur.setFillColor(Color(30, 30, 30, 75));

    game.pausetext.setFont(game.font);
    game.pausetext.setFillColor(Color::Red);
    game.pausetext.setString("Pause");
    game.pausetext.setCharacterSize(90);

    game.gameovertext.setFont(game.font);
    game.gameovertext.setFillColor(Color::Red);
    game.gameovertext.setString("Game Over");
    game.gameovertext.setCharacterSize(90);

    initPlayer(game.player);
    initbear(game.bear);
    initEnemy(game.enemy);
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
            }
            if (event.key.code == Keyboard::Escape) {
                game.pause = !game.pause;
            }
            if (event.key.code == Keyboard::Enter) {
                game.pause = 0;
            }
            if (event.key.code == Keyboard::T) {
                gameover = 1;
                updatePlayer(game.player, game.map, elapsed);
            }
            break;
        }
    }

    game.pausetext.setPosition(game.camera.getCenter() - Vector2f(110, 140));
    game.gameovertext.setPosition(game.camera.getCenter() - Vector2f(150, 140));

    if (game.pause == 0 && gameover == 0) {
        updateBear(game.bear, game.map, elapsed);
        updateEnemy(game.enemy, game.map, elapsed);
        updatePlayer(game.player, game.map, elapsed);
        if (game.enemy.isalive) {
            CollisionPlayerWithEnemy(game.enemy, game.player);
        }
        if (game.bear.isalive) {
            CollisionPlayerWithEnemy(game.bear, game.player);
        }

        int cameraX = round(game.player.hitbox.left + (3 * TILE_WIDTH));
        int cameraY = round(9 * TILE_WIDTH + 10);

        const int left_boundary = 528, right_boundary = 111 * 32;
        if (cameraX < left_boundary)
            cameraX = left_boundary;

        if (cameraX > right_boundary)
            cameraX = right_boundary;



        game.camera.setCenter(cameraX, cameraY); // must be integers or else strange lines appear

        updateLevel(game);
    }
}


void drawGame(Game& game) {
    game.window.setView(game.camera);
    if (game.map.tiles != nullptr) {
        drawLevel(game);
    }

    if (game.bear.isalive == true)
    {
        drawBear(game);
    }
    else {
        game.bear.body.setScale(0, 0);
        game.bear.body.setPosition(0, 0);
    }
    if (game.enemy.isalive == true) {
        drawEnemy(game);
    }
    else{
        game.enemy.body.setScale(Vector2f(0, 0));
        game.enemy.body.setPosition(Vector2f(0, 0));
    }

    drawPlayer(game);

    if (game.pause == 1) {
        game.window.draw(game.blur);
        game.window.draw(game.pausetext);
    }

    if (gameover == 1) {
        game.window.draw(game.blur);
        game.window.draw(game.gameovertext);
    }
    

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

    player.anim[3].spritesheet.loadFromFile("./assets/player/hurt.png");
    initAnimation(player.anim[3], 33, 32, 1);

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
    player.headPoint = FloatRect(player.hitbox.getPosition() + Vector2f(1, -8), Vector2f(30, 8));

    player.debugger = RectangleShape(Vector2f(TILE_WIDTH, 8));
    player.debugger.setFillColor(Color::White);

    player.acceleration.x = 50;

}

void updatePlayer(Player& player, Level& level, float elapsed) {

    if (player.health <= 0)
        gameover = 1;
    player.velocity.y += GRAVITY * elapsed;
    player.velocity.x *= FRICTION;


    if (move_player)
        movePlayer(player, level, elapsed);

    updatePlayerAnimation(player);

    //player.debugger.setPosition(round(player.hitbox.left),round(player.hitbox.top));
    player.body.setTexture(player.anim[player.animationState].spritesheet);
    player.body.setTextureRect(player.anim[player.animationState].currentSprite);
    player.wings.setTextureRect(player.wingsAnimation.currentSprite);
}


void collisionX(Player& player, Level& level) {

    if (player.hitbox.left < 0) {
        player.velocity.x = 0;
        player.hitbox.left = 0;
    }


    int left_tile = player.hitbox.left / TILE_WIDTH;
    int right_tile = left_tile + 1;
    int vertical_pos = player.hitbox.top / TILE_WIDTH;

    int moveDirection = sign(player.velocity.x);

    for (int i = vertical_pos; i <= vertical_pos; i++) {
        for (int j = left_tile; j <= right_tile; j++) {
            Tile& currentTile = level.tiles[j + i * level.width];
            bool collided = player.hitbox.intersects(currentTile.sprite.getGlobalBounds());

            if (currentTile.isPipe && player.canFly && abs(player.velocity.y) > 100 && collided) {
                gameover = 1;
            }


            if (currentTile.isSolid && collided){
                player.velocity.x = 0;
                player.hitbox.left = currentTile.sprite.getPosition().x - TILE_WIDTH * moveDirection;
            }




            if (currentTile.isPowerup && collided) {
                player.canFly = true;
                currentTile.isPowerup = false;
            }

            if (currentTile.isCoin && collided) {
                currentTile.isCoin = false;
                player.score++;
                level.coinBuffer.loadFromFile("./assets/audio/coin.wav");
                level.coinSound.setBuffer(level.coinBuffer);
                level.coinSound.play();
            }

            if (currentTile.isSpike && collided) {

                player.velocity.x = 0;
                player.hitbox.left = currentTile.sprite.getPosition().x - TILE_WIDTH * moveDirection;


            }

            if (currentTile.isFlag && collided) {
                Win = true;  //Change case when flag hit
            }


        }
    }
}

bool collisionY(Player& player, Level& level) { // no head collisions yet

    int above_tile = player.hitbox.top / TILE_WIDTH;
    int below_tile = above_tile + 1;
    float horizontal_pos = player.hitbox.left / TILE_WIDTH;

    if (player.hitbox.top <= 0)
        player.hitbox.top = 0;

    for (int j = horizontal_pos; j <= horizontal_pos + 1; j++) {
        Tile& currentTile = level.tiles[j + below_tile * level.width];
        bool collided = player.hitbox.intersects(currentTile.sprite.getGlobalBounds());

        if (currentTile.isSolid && player.velocity.y >= 0 && collided) {

            player.isJumping = false;
            player.velocity.y = 0;
            player.hitbox.top = currentTile.sprite.getPosition().y - TILE_WIDTH;
            return true;

        }

        if (currentTile.isSpike && player.velocity.y >= 0 && collided) {
            move_player = false;
            player.isJumping = false;
            player.velocity.y = 0;
            player.hitbox.top = currentTile.sprite.getPosition().y - TILE_WIDTH;
            gameover = 1;
        }

    }
    return false;
}

bool collisionHead(Player& player, Level& level) {

    int above_tile = (player.hitbox.top / TILE_WIDTH)-1;
    int below_tile = above_tile + 1;
    float horizontal_pos = player.hitbox.left / TILE_WIDTH; 
    if (above_tile < 0)
        above_tile = 0;
    
    player.headPoint.left = player.hitbox.left + 1;
    player.headPoint.top = player.hitbox.top - 8;

    for (int j = horizontal_pos; j <= horizontal_pos+1; j++) {
        Tile& currentTile = level.tiles[j + above_tile * level.width];

        if (currentTile.isSolid && player.velocity.y <= 0 && currentTile.sprite.getGlobalBounds().intersects(player.headPoint)) {

            player.velocity.y = 0;
            player.headPoint.top = currentTile.sprite.getPosition().y + TILE_WIDTH;
            player.hitbox.top = player.headPoint.top + 8;
            return true;
        }
    }
    return false;
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
    collisionHead(player, level);
    collisionX(player, level);
    collisionY(player, level);
    player.debugger.setPosition(player.headPoint.getPosition());
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

    if (gameover == 1) {
        player.animationState = 3;
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
    level.pipe.loadFromFile("./assets/level/pipe.png");
    level.spikeTexture.loadFromFile("assets/level/spike.png");
    level.flagTexture.loadFromFile("./assets/level/flag.png");

    loadLevelFile(level);

    level.background.setTexture(level.backgroundImage);
    level.background.setColor(Color(230, 230, 230, 255));
    level.background.setScale(3, 3);
    level.background.setOrigin(192, 120); // set origin to center instead of top left

    level.score_text.setFont(font);
    level.score_text.setCharacterSize(32);
    level.score_text.setFillColor(Color::White);

    level.timer_text.setFont(font);
    level.timer_text.setPosition(0, 0);
    level.timer_text.setCharacterSize(32);

    level.win_text.setFont(font);
    level.win_text.setString("YOU WIN!");
    level.win_text.setCharacterSize(100);
    level.win_text.setFillColor(Color::Black);

    level.tiles = new Tile[level.height * level.width];

    for (int i = 0; i < level.height; i++) {
        for (int j = 0; j < level.width; j++) {

            level.tiles[j + i * level.width].sprite.setPosition(j * TILE_WIDTH, i * TILE_WIDTH);
            level.tiles[j + i * level.width].sprite.setTexture(level.spritesheet);
            level.tiles[j + i * level.width].sprite.setScale(2, 2);

            level.tiles[j + i * level.width].coin.setPosition(j * TILE_WIDTH, i * TILE_WIDTH);
            level.tiles[j + i * level.width].coin.setTexture(level.txCoin);
            level.tiles[j + i * level.width].coin.setScale(2, 2);




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
                level.tiles[j + i * level.width].sprite.setTextureRect(IntRect(0, 0, 16, 16));
                level.tiles[j + i * level.width].isPowerup = true;
                break;
            case 'C':
                level.tiles[j + i * level.width].coin.setTextureRect(IntRect(0, 0, 16, 16));
                level.tiles[j + i * level.width].isCoin = true;
                break;
            case 'P':
                level.tiles[j + i * level.width].sprite.setTexture(level.pipe);
                level.tiles[j + i * level.width].sprite.setTextureRect(IntRect(0, 0, 32, 32));
                level.tiles[j + i * level.width].sprite.setScale(1, 1);
                level.tiles[j + i * level.width].isPipe = true;
                level.tiles[j + i * level.width].isSolid = true;
                break;
            case 'T':
                level.tiles[j + i * level.width].sprite.setTexture(level.pipe);
                level.tiles[j + i * level.width].sprite.setTextureRect(IntRect(32, 0, 32, 32));
                level.tiles[j + i * level.width].isPipe = true;
                level.tiles[j + i * level.width].sprite.setScale(1, 1);
                level.tiles[j + i * level.width].isSolid = true;
                break;
            case 'B': 
                level.tiles[j + i * level.width].sprite.setTexture(level.pipe);
                level.tiles[j + i * level.width].sprite.setTextureRect(IntRect(32, 0, 32, 32));
                level.tiles[j + i * level.width].isPipe = true;
                level.tiles[j + i * level.width].sprite.setScale(1, -1);
                level.tiles[j + i * level.width].isSolid = true;
                break;

            case 'S'://spike case
                level.tiles[j + i * level.width].spike.setPosition(j * (TILE_WIDTH), i * (TILE_WIDTH));//position of spike
                level.tiles[j + i * level.width].spike.setScale(1, 1);
                level.tiles[j + i * level.width].spike.setTexture(level.spikeTexture);
                level.tiles[j + i * level.width].isSpike = true;
                break;

            case 'W':
                level.tiles[j + i * level.width].flag.setPosition(j * TILE_WIDTH, i * TILE_WIDTH);
                level.tiles[j + i * level.width].flag.setTexture(level.flagTexture);
                level.tiles[j + i * level.width].flag.setTextureRect(IntRect(0, 0, 32, 32));
                level.tiles[j + i * level.width].isFlag = true; //flag as a tile
                break;
            case '<':
                level.tiles[j + i * level.width].flag.setPosition(j * TILE_WIDTH, i * TILE_WIDTH);
                level.tiles[j + i * level.width].flag.setTexture(level.flagTexture);
                level.tiles[j + i * level.width].flag.setTextureRect(IntRect(32,0,32,32));
                level.tiles[j + i * level.width].isFlag = true; //flag as a tile
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
    game.map.score_text.setPosition(round(game.camera.getCenter().x-75), 0);
    game.map.score_text.setString("SCORE\n   " + std::to_string(game.player.score));

    if (game.map.timer.getElapsedTime().asSeconds() >= 1) {
        if (Win != true) // stopping timer
            game.map.countdown_timer--;
        game.map.countdown_timer;
        game.map.timer_text.setString("TIME\n " + std::to_string(game.map.countdown_timer));
        game.map.timer.restart();
        if (game.map.countdown_timer <= 0)
            game.window.close();
    }

    game.map.timer_text.setPosition(round(game.camera.getCenter().x + 200), 0);


    game.map.background.setPosition(game.camera.getCenter());

    if (Win) {
        game.map.score_text.setFillColor(Color::Black);
        game.map.timer_text.setFillColor(Color::Black);
        game.map.win_text.setPosition(round(game.camera.getCenter().x) - 200, game.camera.getCenter().y - 150);
        game.map.timer_text.setPosition(round(game.camera.getCenter().x + 50), game.camera.getCenter().y - 50);
        game.map.score_text.setPosition(round(game.camera.getCenter().x) - 200, game.camera.getCenter().y - 50);
    }


}

void drawLevel(Game& game) {
    game.window.draw(game.map.background);
    for (int i = 0; i < game.map.height; i++) {
        for (int j = 0; j < game.map.width; j++) {

            Tile &currentTile = game.map.tiles[j + i * game.map.width];
            if (currentTile.isSolid || currentTile.isPowerup || currentTile.isPipe) { //make sure sprite isn't empty
                game.window.draw(game.map.tiles[j + i * game.map.width].sprite);
            }

            if (currentTile.isCoin) {
                game.window.draw(game.map.tiles[j + i * game.map.width].coin);
            }

            if (currentTile.isSpike){
                game.window.draw(game.map.tiles[j + i * game.map.width].spike);//drawing spike
            }

            if (currentTile.isFlag) {
                game.window.draw(game.map.tiles[j + i * game.map.width].flag);
            }
                

        }
    }
    game.window.draw(game.map.timer_text);
    game.window.draw(game.map.score_text);

    if (Win)
    {
        //Drawing win text alone in case of winning
        game.window.draw(game.map.win_text);

    }

    
}


void makeMenu(Menu& menu, float width, float height) {
    menu.font.loadFromFile("./assets/font.ttf");

    menu.mainmenu[0].setFont(menu.font);
    menu.mainmenu[0].setFillColor(Color::Red);
    menu.mainmenu[0].setString("Play");
    menu.mainmenu[0].setCharacterSize(90);
    menu.mainmenu[0].setPosition(Vector2f(width / 2 - 100, height / 4 - 100));

    menu.mainmenu[1].setFont(menu.font);
    menu.mainmenu[1].setFillColor(Color::White);
    menu.mainmenu[1].setString("Instructions");
    menu.mainmenu[1].setCharacterSize(90);
    menu.mainmenu[1].setPosition(Vector2f(width / 2 - 100, height / 4 + 100));

    menu.mainmenu[2].setFont(menu.font);
    menu.mainmenu[2].setFillColor(Color::White);
    menu.mainmenu[2].setString("Exit");
    menu.mainmenu[2].setCharacterSize(90);
    menu.mainmenu[2].setPosition(Vector2f(width / 2 - 100, height / 4 + 300));

}
void drawMenu(Menu& menu, RenderWindow& window) {
    for (int i = 0; i < 3; i++)
    {
        window.draw(menu.mainmenu[i]);
    }

}
void movedown(Menu& menu) {
    if (menu.selected + 1 <= 3)
    {
        menu.mainmenu[menu.selected].setFillColor(Color::White);
        menu.selected++;
        if (menu.selected == 3) {
            menu.selected = 0;
        }
        menu.mainmenu[menu.selected].setFillColor(Color::Red);
    }
}
void moveup(Menu& menu) {
    if (menu.selected - 1 >= -1)
    {
        menu.mainmenu[menu.selected].setFillColor(Color::White);
        menu.selected--;
        if (menu.selected == -1) {
            menu.selected = 2;
        }
        menu.mainmenu[menu.selected].setFillColor(Color::Red);
    }
}

void pname(RenderWindow& window, std::string& name) {
    if (!name.empty()) {
        name.clear();
    }
    Texture bgmenu;
    bgmenu.loadFromFile("background main menu1.jpg");
    Sprite bg;
    bg.setTexture(bgmenu);
    Font font;
    font.loadFromFile("font.ttf");
    Text t1;
    Text t2;
    t1.setFont(font);
    t2.setFont(font);
    t1.setString("Enter your name ");
    t1.setCharacterSize(70);
    t2.setCharacterSize(70);
    t1.setPosition(10, 10);
    t2.setPosition(10, 100);
    t1.setFillColor(Color::Red);
    t2.setFillColor(Color::Red);
    while (window.isOpen()) {
        Event event;
        while (window.pollEvent(event))
        {
            if (event.type == Event::Closed) {
                window.close();
                break;
            }
            if (event.type == Event::TextEntered) {
                name += static_cast<char>(event.text.unicode);
            }
            if (Keyboard::isKeyPressed(Keyboard::BackSpace) && name.size() > 0) {
                name.resize(name.size() - 1);
            }
            if (Keyboard::isKeyPressed(Keyboard::Escape)) {
                pagenum = 10; // padge number of main menu
                return;
            }
            if (Keyboard::isKeyPressed(Keyboard::Return) && name.size() > 1) {
                pagenum = 3;// padge numper of game
                //return;
            }
        }

        t2.setString(name);
        window.clear();
        window.draw(bg);
        window.draw(t1);
        window.draw(t2);
        window.display();
    }
}

void updateMenu(Menu& menu, RenderWindow& window) {
    Event event;
    std::string name;

    while (window.pollEvent(event))
    {
        if (event.type == Event::Closed) {
            window.close();
            break;
        }
        if (event.type == Event::KeyPressed) {
            if (event.key.code == Keyboard::Up) {
                moveup(menu);
            }
            if (event.key.code == Keyboard::Down) {
                movedown(menu);
            }
            if (event.key.code == Keyboard::Enter) {
                pagenum = menu.selected;
            }
        }

    }
    //std::cout << menu.selected << ' ' << pagenum << endl;

}


void initbear(Bear& bear) {
    if (bear.anim[0].spritesheet.loadFromFile("./assets/enemy/bear.png")) {
        std::cout << "bear loaded ";
    }
    initAnimation(bear.anim[0], 54, 63, 4);
    bear.anim[0].flipped = false; //sprite faces left, movement is right by default. 
    bear.velocity.x = 65;
    bear.health = 500;
    bear.body.setPosition(Vector2f(100, 0));
    bear.body.setScale(Vector2f(2, 2));
}
void initEnemy(Enemy& enemy)
{
    if (enemy.anim[0].spritesheet.loadFromFile("./assets/enemy/rat.png")) {
        std::cout << "rat loaded ";
    }
    initAnimation(enemy.anim[0], 36, 28, 6);
    enemy.anim[0].flipped = true; //sprite faces left, movement is right by default.
    enemy.velocity.x = 45;
    enemy.health = 35;
    enemy.body.setPosition(Vector2f(750, 400));
    enemy.body.setScale(Vector2f(2, 2));
}

void movebear(Bear& bear, Level level, float elapsed) {
    bear.velocity.y += GRAVITY * elapsed;
    if (bear.velocity.y > 500)
        bear.velocity.y = 500;
    bear.body.move(bear.velocity.x * elapsed, bear.velocity.y * elapsed);
    collisionX(bear, level);
    collisionY(bear, level);
}

void moveEnemy(Enemy& enemy, Level level, float elapsed) {
    //Apply gravity
    enemy.velocity.y += GRAVITY * elapsed;

    // Make sure enemy doesn't go too fast, otherwise collisions will break
    if (enemy.velocity.y > 500)
        enemy.velocity.y = 500;

    // Move horizontally and vertically
    enemy.body.move(enemy.velocity.x * elapsed, enemy.velocity.y * elapsed);

    // Check for collisions in X direction
    collisionX(enemy, level);
    collisionY(enemy, level);
}


void updateBear(Bear& bear, Level& level, float elapsed) {
    if (bear.health <= 0)
        bear.isalive = false;
    updatebearAnimation(bear);
    movebear(bear, level, elapsed);
    bear.body.setTexture(bear.anim[bear.animationState].spritesheet);
    bear.body.setTextureRect(bear.anim[bear.animationState].currentSprite);
}

void updateEnemy(Enemy& enemy, Level& level, float elapsed) {

    // Update enemy animation
    updateEnemyAnimation(enemy);

    // Move the enemy
    moveEnemy(enemy, level, elapsed);

    // setting texture

    enemy.body.setTexture(enemy.anim[enemy.animationState].spritesheet);
    enemy.body.setTextureRect(enemy.anim[enemy.animationState].currentSprite);
}

void updatebearAnimation(Bear& bear) {
    updateAnimation(bear.anim[0]);
}

void updateEnemyAnimation(Enemy& enemy)
{
    updateAnimation(enemy.anim[0]);

}
void collisionX(Bear& bear, Level& level) {
    int left_tile = bear.body.getPosition().x / TILE_WIDTH;
    int right_tile = left_tile + 2;
    int vertical_pos = bear.body.getPosition().y / TILE_WIDTH;
    //Sprite is approximately 3 tiles long (when rounded up) so we need to check the 3 tiles it covers horizontally
    //Since we're dealing with X collisions we don't need to check below it

    int moveDirection = sign(bear.velocity.x);

    for (int j = left_tile; j <= right_tile; j++) {
        Tile& currentTile = level.tiles[j + vertical_pos * level.width];

        if (currentTile.isSolid && bear.body.getGlobalBounds().intersects(currentTile.sprite.getGlobalBounds())) {
            bear.velocity.x *= -1;
            bear.anim[0].flipped = !bear.anim[0].flipped;
            bear.body.setPosition(currentTile.sprite.getPosition().x - bear.anim->frameWidth * 2 * moveDirection, bear.body.getPosition().y);
        }
    }
}
void collisionX(Enemy& enemy, Level& level)
{
    int left_tile = enemy.body.getPosition().x / TILE_WIDTH;
    int right_tile = left_tile + 2;
    int vertical_pos = enemy.body.getPosition().y / TILE_WIDTH;
    //Sprite is approximately 3 tiles long (when rounded up) so we need to check the 3 tiles it covers horizontally
    //Since we're dealing with X collisions we don't need to check below it

    int moveDirection = sign(enemy.velocity.x);

    for (int j = left_tile; j <= right_tile; j++) {
        Tile& currentTile = level.tiles[j + vertical_pos * level.width];

        if (currentTile.isSolid && enemy.body.getGlobalBounds().intersects(currentTile.sprite.getGlobalBounds())) {
            enemy.velocity.x *= -1;
            enemy.anim[0].flipped = !enemy.anim[0].flipped;
            enemy.body.setPosition(currentTile.sprite.getPosition().x - enemy.anim->frameWidth * 2 * moveDirection, enemy.body.getPosition().y);
        }
    }
}

void collisionY(Bear& bear, Level& level) {
    int above_tile = bear.body.getPosition().y / TILE_WIDTH;
    int below_tile = above_tile + 3;
    float horizontal_pos = bear.body.getPosition().x / TILE_WIDTH;
    if (below_tile < 0)
        below_tile = 0;
    //for Y collisions we need to check the tiles it covers horizontally and the ones below


    for (int i = above_tile; i < below_tile+3; i++) {
        for (int j = horizontal_pos; j <= horizontal_pos + 3; j++) {
            Tile& currentTile = level.tiles[j + i * level.width];

            if (currentTile.isSolid && bear.velocity.y >= 0 && bear.body.getGlobalBounds().intersects(currentTile.sprite.getGlobalBounds())) {

                bear.velocity.y = 0;
                bear.body.setPosition(bear.body.getPosition().x, currentTile.sprite.getPosition().y - (63 * 2) );
                //Push it out of the block vertically by the height of the sprite
            }
        }
    }

}

void collisionY(Enemy& enemy, Level& level) { // no head collisions for enemy

    int above_tile = enemy.body.getPosition().y / TILE_WIDTH;
    int below_tile = above_tile + 3;
    float horizontal_pos = enemy.body.getPosition().x / TILE_WIDTH;
    if (below_tile < 0)
        below_tile = 0;
    //for Y collisions we need to check the tiles it covers horizontally and the ones below


    for (int i = above_tile; i < below_tile; i++) {
        for (int j = horizontal_pos; j <= horizontal_pos + 2; j++) {
            Tile& currentTile = level.tiles[j + i * level.width];

            if (currentTile.isSolid && enemy.velocity.y >= 0 && enemy.body.getGlobalBounds().intersects(currentTile.sprite.getGlobalBounds())) {

                enemy.velocity.y = 0;
                enemy.body.setPosition(enemy.body.getPosition().x, currentTile.sprite.getPosition().y - (28 * 2));
                //Push it out of the block vertically by the height of the sprite
            }
        }
    }
}

void CollisionPlayerWithEnemy(Enemy& enemy, Player& player)
{
    if (player.hitbox.top < enemy.body.getPosition().y && player.hitbox.intersects(enemy.body.getGlobalBounds())) {
        player.hitbox.top = enemy.body.getPosition().y - 32;
        player.velocity.y = -500;
        enemy.isalive = false;
        player.score += 10;
    }
    else if (player.hitbox.intersects(enemy.body.getGlobalBounds())) {
        int moveDirection = sign(player.velocity.x);
        player.hitbox.left = enemy.body.getPosition().x - 32 * moveDirection;
        player.velocity.x = 500 * -moveDirection;
        player.health--;
    }
}

void CollisionPlayerWithEnemy(Bear& bear, Player& player)
{
    if (player.velocity.y > 0 && player.hitbox.intersects(bear.body.getGlobalBounds())) {
        player.hitbox.top = bear.body.getPosition().y - 32;
        player.velocity.y = -900;
        bear.health -= 100;
        player.score += 10;
    }

    else if (abs(player.velocity.x > 0) && player.hitbox.intersects(bear.body.getGlobalBounds())) {
        std::cout << "X Collision\n";
        int moveDirection = sign(player.velocity.x);
        player.hitbox.left = bear.body.getPosition().x - 32 * moveDirection;
        player.velocity.x = 500 * -moveDirection;
        player.health--;
    }
}



void drawEnemy(Game& game) {
    // std::cout << "Drawing enemy..." << std::endl;
    // std::cout << "Enemy Position: " << game.enemy.body.getPosition().x << ", " << game.enemy.body.getPosition().y << std::endl;
    if (game.enemy.isalive == true) {
        game.window.draw(game.enemy.body);
    }
}
void drawBear(Game& game) {
    if (game.bear.isalive == true) {
        game.window.draw(game.bear.body);
    }
}