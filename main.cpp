#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <string>
#include <fstream> 
using namespace sf;

const float TILE_WIDTH = 32;
const int LEVEL_TIMER = 60;

bool move_player = true;
bool Win = false;
bool gameover = 0;
bool secondLevelUnlocked = false;

const float FRICTION = 0.91f;
const float GRAVITY = 2200.0f;
const float TERMINAL_VELOCITY = 500.0f; //maximum vertical velocity


int sign(const int& num) {
    return (num > 0 ? 1 : -1);
}
int pagenum = 10;

struct Menu {
    Text mainmenu[3];
    Text instructionsText;
    Font font;
    int selected = 0;
    Sprite background;
    Texture backgroundTexture;
};

struct Projectile {
    Texture bullets_tx;
    Sprite bullets_sprite;
    int direction; // 1-->right , 2-->left
    int last_key;
    float velocity = 0;
    bool together = true;
    SoundBuffer bulletSoundBuffer;
    Sound bulletSound;
    bool soundPlay = true;
};

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
    bool isBox = false;
    bool isSuper = false;
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
    Clock jumpTimer;
};

struct Level {
    int width = 128;
    int height = 32;

    Clock timer;
    int countdown_timer = 60;

    std::string map_string;

    Texture spritesheet;
    Texture feather;
    Texture txCoin;
    Texture pipe;
    Texture spikeTexture;
    Texture flagTexture;
    Texture boxTexture;
    Texture superTexture;

    Tile* tiles = nullptr; //dynamic array
    Texture backgroundImage;
    Sprite background;

    Text score_text;
    Text timer_text;
    Text win_text;

    SoundBuffer coinBuffer;
    SoundBuffer powerBuffer;
    SoundBuffer winBuffer;//win buffer

    Sound coinSound;
    Sound powerSound;
    Sound winSound;//win sound
    bool playWinSound = 1;

    Enemy enemy;
    Bear bear;
};

struct Player {
    Sprite body;
    FloatRect hitbox;
    FloatRect headPoint;

    Animation anim[4];
    Animation wingsAnimation;
    Sprite wings;
    int animationState = 0; // 0 for idle, 1 for running, 2 for jumping

    int score = 0;
    int health = 3;
    int lives = 1;
    int damageMultiplier = 1;

    Projectile bullet;

    SoundBuffer hurtBuffer;
    SoundBuffer jumpBuffer;
    SoundBuffer gameoverBuffer;

    Sound hurtSound;
    Sound jumpSound;
    Sound gameoverSound;


    Vector2i velocity;
    Vector2i acceleration;
    float maxVelocity = 300.0f;
    float minVelocity = 20.0f;
    bool isJumping = false;
    bool canFly = false;
    bool canThrow = true;
};

struct Game {
    RenderWindow window;
    View camera;
    Level map[2];
    int currentMap = 0;
    Player player;
    Font font;

    bool pause = 0;
    Text pausetext;
    Text gameovertext;
    RectangleShape blur;
};

void makeMenu(Menu& menu, float width, float height);
void updateMenu(Menu& menu, RenderWindow& window);
void drawMenu(Menu& menu, RenderWindow& window);
void movedown(Menu& menu);
void moveup(Menu& menu);

void initGame(Game& game);
void updateGame(Game& game, float elapsed);
void drawGame(Game& game);

void initPlayer(Player& player);
void movePlayer(Player& player, Level& level, float elapsed);
void updatePlayer(Player& player, Level& level, float elapsed);
void updatePlayerAnimation(Player& player);
void collisionX(Player& player, Level& level);
void collisionX(Projectile& bullet, Level& level, Player& player);
void collisionY(Player& player, Level& level);
bool collisionHead(Player& player, Level& level);
void drawPlayer(Game& game);

void initEnemy(Enemy& enemy, Vector2f position);
void moveEnemy(Enemy& enemy, Level level, float elapsed);
void updateEnemy(Enemy& enemy, Level& level, float elapsed);
void updateEnemyAnimation(Enemy& player);
void collisionX(Enemy& enemy, Level& level);
void collisionY(Enemy& enemy, Level& level);
void drawEnemy(Game& game);
void CollisionPlayerWithEnemy(Enemy& enemy, Player& player);

void initbear(Bear& bear, Vector2f position);
void movebear(Bear& bear, Level level, float elapsed);
void updateBear(Bear& bear, Level& level, float elapsed);
void updatebearAnimation(Bear& bear);
void drawBear(Game& game);
void collisionX(Bear& bear, Level& level);
void collisionY(Bear& bear, Level& level);
void CollisionPlayerWithEnemy(Bear& bear, Player& player);

void initAnimation(Animation& anim, int width, int height, int frameCount);
void updateAnimation(Animation& anim);

void initLevel(Level& level, Font &font,int i);
void loadLevelFile(Level& level,int i);
void updateLevel(Game& game,float elapsed);
void drawLevel(Game& game);


int main() {
    std::string name;
    Menu menu;
    Game game;
    initGame(game);
    makeMenu(menu, 1056, 594);
    
    Clock clock;

    while (game.window.isOpen()) {
        switch (pagenum) {
        case 0:
            if (clock.getElapsedTime().asSeconds() > 0.2f)
                clock.restart();
            updateGame(game, clock.restart().asSeconds());
            game.window.clear(Color::Black);
            drawGame(game);
            break;
        case 1:
            
        case 10:
            updateMenu(menu, game.window);
            game.window.clear(Color::Black);
            drawMenu(menu, game.window);
            break;
        case 2:
            game.window.close();
            break;
        }
        game.window.display();
    }

    for (int i = 0; i < 2; i++) {
        delete[] game.map[i].tiles;
        game.map[i].tiles = nullptr;
    }
    return 0;
}


void makeMenu(Menu& menu, float width, float height) {
    menu.font.loadFromFile("./assets/font.ttf");

    menu.mainmenu[0].setFont(menu.font);
    menu.mainmenu[0].setFillColor(Color(85, 140, 250, 255));
    menu.mainmenu[0].setString("Play");
    menu.mainmenu[0].setCharacterSize(96);
    menu.mainmenu[0].setOrigin(menu.mainmenu[0].getLocalBounds().width/2, menu.mainmenu[0].getLocalBounds().height/2);
    menu.mainmenu[0].setPosition(Vector2f(1536/2, 864/2-150));

    menu.mainmenu[1].setFont(menu.font);
    menu.mainmenu[1].setFillColor(Color::White);
    menu.mainmenu[1].setString("Instructions");
    menu.mainmenu[1].setCharacterSize(96);
    menu.mainmenu[1].setOrigin(menu.mainmenu[1].getLocalBounds().width / 2, menu.mainmenu[1].getLocalBounds().height / 2);
    menu.mainmenu[1].setPosition(Vector2f(1536 / 2+10, 864 / 2));

    menu.mainmenu[2].setFont(menu.font);
    menu.mainmenu[2].setFillColor(Color::White);
    menu.mainmenu[2].setString("Exit");
    menu.mainmenu[2].setCharacterSize(96);
    menu.mainmenu[2].setOrigin(menu.mainmenu[2].getLocalBounds().width / 2, menu.mainmenu[2].getLocalBounds().height / 2);
    menu.mainmenu[2].setPosition(Vector2f(1536/2, 864 / 2 + 150));

    menu.backgroundTexture.loadFromFile("./assets/background.png");
    menu.background.setTexture(menu.backgroundTexture);

    menu.instructionsText.setFont(menu.font);
    menu.instructionsText.setString(
        "A, D - Move left and right\nW - Jump\nSpace - Fly\nF - Throw Projectile" 
    );
    menu.instructionsText.setFillColor(Color::White);
    menu.instructionsText.setCharacterSize(48);
    menu.instructionsText.setOrigin(menu.instructionsText.getLocalBounds().width / 2, menu.instructionsText.getLocalBounds().height / 2);
    menu.instructionsText.setPosition(1536 / 2, 864 / 2);

}
void drawMenu(Menu& menu, RenderWindow& window) {
    window.setView(window.getDefaultView());
    window.draw(menu.background);
    if (pagenum == 10) {
        for (int i = 0; i < 3; i++) {
        window.draw(menu.mainmenu[i]);
    }
    }
    else if (pagenum == 1) {
        window.draw(menu.instructionsText);
    }

}
void movedown(Menu& menu) {

	menu.mainmenu[menu.selected].setFillColor(Color::White);
	menu.selected++;
	if (menu.selected > 2) 
		menu.selected = 0;
	menu.mainmenu[menu.selected].setFillColor(Color(85, 140, 250, 255));
}
void moveup(Menu& menu) {

	menu.mainmenu[menu.selected].setFillColor(Color::White);
	menu.selected--;
	if (menu.selected < 0) 
		menu.selected = 2;
	menu.mainmenu[menu.selected].setFillColor(Color(85, 140, 250, 255));

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
            if (pagenum == 1 && event.key.code == Keyboard::Escape) {
                pagenum = 10;
            }
        }

    }
    
}

void initGame(Game& game) {
    game.window.create(VideoMode(1536, 864), "Super Hobs");
    game.window.setFramerateLimit(90);

    Image icon;
    icon.loadFromFile("./assets/icon.png");
    game.window.setIcon(128, 124, icon.getPixelsPtr());

    game.camera.setCenter(Vector2f(350.f, 300.f));
    game.camera.setSize(Vector2f(1056, 594));

    game.font.loadFromFile("./assets/font.ttf");
    game.font.setSmooth(false);

    game.blur.setPosition(Vector2f(0, 0));
    game.blur.setScale(Vector2f(3, 3));
    game.blur.setSize(Vector2f(1536, 875));
    game.blur.setFillColor(Color(30, 30, 30, 75));

    game.pausetext.setFont(game.font);
    game.pausetext.setFillColor(Color::White);
    game.pausetext.setString("PAUSED");
    game.pausetext.setCharacterSize(96);

    game.gameovertext.setFont(game.font);
    game.gameovertext.setFillColor(Color::Red);
    game.gameovertext.setString("GAME OVER");
    game.gameovertext.setCharacterSize(96);
    game.gameovertext.setOutlineColor(Color::White);
    game.gameovertext.setOutlineThickness(2);

    initPlayer(game.player);
    initLevel(game.map[0], game.font, 1);
    initLevel(game.map[1], game.font, 2);
}
void updateGame(Game& game, float elapsed) {
    Event event;
    while (game.window.pollEvent(event))
    {
        switch (event.type) {
        case Event::Closed:
            game.window.close(); break;

        case Event::KeyPressed:
            if (event.key.scancode == Keyboard::Scan::W && !game.player.isJumping) {
                game.player.velocity.y = -800.0f;
                game.player.jumpSound.play();
            }
            if (event.key.code == Keyboard::Escape && !gameover && !Win) {
                game.pause = !game.pause;
            }
            if (event.key.code == Keyboard::Enter) {
                game.pause = 0;
            }

            if (gameover) {
                if (event.key.code == Keyboard::R) {
                    if (game.player.lives == 0) {
                        pagenum = 10;
                    }
                    game.player.hitbox.top = 0;
                    game.player.hitbox.left = 0;
                    initLevel(game.map[game.currentMap], game.font, game.currentMap + 1);
                    gameover = 0;
                    move_player = 1;
                    game.player.health = 3;
                }
                if (event.key.code == Keyboard::Escape) {
                    game.player.hitbox.top = 0;
                    game.player.hitbox.left = 0;
                    initLevel(game.map[game.currentMap], game.font, game.currentMap + 1);
                    gameover = 0;
                    move_player = 1;
                    pagenum = 10;
                    game.player.health = 3;
                }

            }

            if (Win) {
                if (event.key.code == Keyboard::Escape) {
                    game.player.hitbox.top = 0;
                    game.player.hitbox.left = 0;
                    initLevel(game.map[game.currentMap], game.font, game.currentMap + 1);
                    gameover = 0;
                    pagenum = 10;
                    Win = false;
                }
                if (event.key.code == Keyboard::Enter && game.currentMap == 0) {
                    game.currentMap = 1;
                    game.player.hitbox.left = 0;
                    game.player.hitbox.top = 0;
                    Win = 0;
                }
            }
            break;
        }
    }

    game.pausetext.setPosition(game.camera.getCenter() - Vector2f(140, 140));
    game.gameovertext.setPosition(game.camera.getCenter() - Vector2f(190, 140));

    if (game.pause == 0 && gameover == 0) {
        updatePlayer(game.player, game.map[game.currentMap], elapsed);

        if (game.map[game.currentMap].enemy.isalive) {
            CollisionPlayerWithEnemy(game.map[game.currentMap].enemy, game.player);
        }
        if (game.map[game.currentMap].bear.isalive) {
            CollisionPlayerWithEnemy(game.map[game.currentMap].bear, game.player);
        }

        int cameraX = round(game.player.hitbox.left + (3 * TILE_WIDTH));
        int cameraY = round(9 * TILE_WIDTH + 10);

        const int left_boundary = 528, right_boundary = 111 * 32;
        if (cameraX < left_boundary)
            cameraX = left_boundary;

        if (cameraX > right_boundary)
            cameraX = right_boundary;



        game.camera.setCenter(cameraX, cameraY); // must be integers or else strange lines appear

        updateLevel(game,elapsed);
    }

}
void drawGame(Game& game) {
    game.window.setView(game.camera);

    if (game.map[game.currentMap].tiles != nullptr) {
        drawLevel(game);
    }

    if (game.map[game.currentMap].bear.isalive == true) {
        drawBear(game);
    }

    if (game.map[game.currentMap].enemy.isalive == true) {
        drawEnemy(game);
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
    player.hurtBuffer.loadFromFile("./assets/audio/hurt.wav");
    player.hurtSound.setBuffer(player.hurtBuffer);

    player.jumpBuffer.loadFromFile("./assets/audio/jump.wav");
    player.jumpSound.setBuffer(player.jumpBuffer);
    player.jumpSound.setVolume(30);

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

    player.acceleration.x = 50;

    player.bullet.bullets_tx.loadFromFile("./assets/player/projectile.png");
    player.bullet.bullets_sprite.setTexture(player.bullet.bullets_tx);
    player.bullet.bullets_sprite.setScale(0.8, 0.8);

    player.canFly = false;
}
void updatePlayer(Player& player, Level& level, float elapsed) {
    if (player.health <= 0) {
        gameover = 1;
    }
    if (player.lives <= 0) {
        secondLevelUnlocked = false;
        player.lives = 1;
        player.score = 0;
    }

    player.velocity.y += GRAVITY * elapsed;
    player.velocity.x *= FRICTION;

    collisionX(player.bullet, level, player);

    if (player.score >= 20) {
        player.score = 0;
        player.lives++;
    }


    if (move_player)
        movePlayer(player, level, elapsed);

    updatePlayerAnimation(player);

    player.body.setTexture(player.anim[player.animationState].spritesheet);
    player.body.setTextureRect(player.anim[player.animationState].currentSprite);
    player.wings.setTextureRect(player.wingsAnimation.currentSprite);
    if (player.bullet.together)
        player.bullet.bullets_sprite.setPosition(player.hitbox.getPosition().x + 9, player.hitbox.getPosition().y);
    else {
        if (player.bullet.last_key == 1)
            player.bullet.bullets_sprite.move(player.bullet.velocity, 0);
        if (player.bullet.last_key == 2)
            player.bullet.bullets_sprite.move(-1 * player.bullet.velocity, 0);

        if (player.bullet.soundPlay) {
            player.bullet.bulletSound.play();
            player.bullet.soundPlay = false;
        }
    }
}
void collisionX(Player& player, Level& level) {

    if (player.hitbox.left < 0) {
        player.velocity.x = 0;
        player.hitbox.left = 0;
    }

    if (player.hitbox.left > 127*32) {
        player.velocity.x = 0;
        player.hitbox.left = 127*32;
    }


    int left_tile = player.hitbox.left / TILE_WIDTH;
    int right_tile = left_tile + 1;
    int vertical_pos = player.hitbox.top / TILE_WIDTH;

    int moveDirection = sign(player.velocity.x);

	for (int j = left_tile; j <= right_tile; j++) {
		Tile& currentTile = level.tiles[j + vertical_pos * level.width];
		bool collided = player.hitbox.intersects(currentTile.sprite.getGlobalBounds());

		if (currentTile.isPipe && player.canFly && abs(player.velocity.y) > 100 && collided) {
			gameover = 1;
		}


		if (currentTile.isSolid && collided) {
			player.velocity.x = 0;
			player.hitbox.left = currentTile.sprite.getPosition().x - TILE_WIDTH * moveDirection;
		}

		if (currentTile.isPowerup && !player.canFly && collided) {
			level.powerBuffer.loadFromFile("./assets/audio/powerup.wav");
			level.powerSound.setBuffer(level.powerBuffer);
			level.powerSound.play();
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

		if (currentTile.isFlag && !level.bear.isalive && collided) {
			Win = true;  //Change case when flag hit
		}

		if (currentTile.isSuper && collided) {
			level.powerBuffer.loadFromFile("./assets/audio/powerup.wav");
			level.powerSound.setBuffer(level.powerBuffer);
			level.powerSound.play();
			player.damageMultiplier = 3;
			player.bullet.bullets_tx.loadFromFile("./assets/player/projectile2.png");
			currentTile.isSuper = false;
		}
  
    }
}
void collisionX(Projectile& bullet, Level& level, Player& player) {
    int left_tile = bullet.bullets_sprite.getPosition().x / TILE_WIDTH;
    int right_tile = left_tile + 1;
    int vertical_pos = bullet.bullets_sprite.getPosition().y / TILE_WIDTH;

    //int moveDirection = sign(player.velocity.x);


    for (int i = vertical_pos; i <= vertical_pos; i++) {
        for (int j = left_tile; j <= right_tile; j++) {
            Tile& currentTile = level.tiles[j + i * level.width];

            if (currentTile.isSolid && bullet.bullets_sprite.getGlobalBounds().intersects(currentTile.sprite.getGlobalBounds())) {
                bullet.together = true;
                player.canThrow = true;
                player.bullet.soundPlay = true;
            }

        }
    }
}
void collisionY(Player& player, Level& level) { // no head collisions yet
    int above_tile = player.hitbox.top / TILE_WIDTH;
    int below_tile = above_tile + 1;
    float horizontal_pos = player.hitbox.left / TILE_WIDTH;

    if (player.hitbox.top <= 0)
        player.hitbox.top = 0;

    if (player.hitbox.top >= 21 * 32) 
        gameover = true;
    

    for (int j = horizontal_pos; j <= horizontal_pos + 1; j++) {
        Tile& currentTile = level.tiles[j + below_tile * level.width];
        bool collided = player.hitbox.intersects(currentTile.sprite.getGlobalBounds());

        if (currentTile.isSolid && player.velocity.y >= 0 && collided) {

            player.isJumping = false;
            player.velocity.y = 0;
            player.hitbox.top = currentTile.sprite.getPosition().y - TILE_WIDTH;

        }

        if (currentTile.isSpike && player.velocity.y >= 0 && collided) {
            move_player = false;
            player.isJumping = false;
            player.velocity.y = 0;
            player.hitbox.top = currentTile.sprite.getPosition().y - TILE_WIDTH;
            gameover = 1;
        }

    }
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
        if (currentTile.isBox && player.velocity.y <= 0 && currentTile.sprite.getGlobalBounds().intersects(player.headPoint)) {
            player.velocity.y = 0;
            player.headPoint.top = currentTile.sprite.getPosition().y + TILE_WIDTH;
            player.hitbox.top = player.headPoint.top + 8;
            currentTile.isBox = false;
            currentTile.isSolid = false;
            currentTile.sprite.setTexture(level.feather);
            currentTile.sprite.setTextureRect(IntRect(0,0,16,16));
            currentTile.isPowerup = true;
        }

        if (currentTile.isSolid && player.velocity.y <= 0 && currentTile.sprite.getGlobalBounds().intersects(player.headPoint)) {

            player.velocity.y = 0;
            player.headPoint.top = currentTile.sprite.getPosition().y + TILE_WIDTH;
            player.hitbox.top = player.headPoint.top + 8;
        }

    }
    return false;
}
void drawPlayer(Game& game) {
    if (game.player.canFly)
        game.window.draw(game.player.wings);

    game.window.draw(game.player.bullet.bullets_sprite);
    
    game.window.draw(game.player.body);
}
void movePlayer(Player& player, Level& level, float elapsed) {

    if (Keyboard::isKeyPressed(Keyboard::Scan::D)) {
        for (int i = 0; i < 3; i++) {
            player.anim[i].flipped = 0;
        }
        player.bullet.direction = 1;
        player.wingsAnimation.flipped = 0;

        player.velocity.x += player.acceleration.x;
    }

    if (Keyboard::isKeyPressed(Keyboard::Scan::A)) {
        for (int i = 0; i < 3; i++){
            player.anim[i].flipped = 1;
        }
            player.wingsAnimation.flipped = 1;

        player.bullet.direction = 2;
        player.velocity.x += player.acceleration.x * -1;
    }

    if (Keyboard::isKeyPressed(Keyboard::Scan::Space) && player.canFly) {
        player.velocity.y = -400;
    }

    if (Keyboard::isKeyPressed(Keyboard::Scan::F) && player.canThrow) {
        player.canThrow = false;
        player.bullet.together = false;
        player.bullet.velocity = 7;
        player.bullet.last_key = player.bullet.direction;
        player.bullet.bulletSoundBuffer.loadFromFile("./assets/audio/shoot.wav");
        player.bullet.bulletSound.setBuffer(player.bullet.bulletSoundBuffer);
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
        player.gameoverBuffer.loadFromFile("./assets/audio/lose.wav");
        player.gameoverSound.setBuffer(player.gameoverBuffer);
        player.gameoverSound.play();   //Whenever animation of losing occurs, play lost sound
        player.lives--;
    }
}

void initEnemy(Enemy& enemy,Vector2f position)
{
    enemy.anim[0].spritesheet.loadFromFile("./assets/enemy/rat.png");
    initAnimation(enemy.anim[0], 36, 28, 6);
    enemy.anim[0].flipped = true; //sprite faces left, movement is right by default.
    enemy.velocity.x = 45;
    enemy.health = 35;
    enemy.body.setPosition(position);
    enemy.body.setScale(Vector2f(2, 2));
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
void updateEnemy(Enemy& enemy, Level& level, float elapsed) {

    // Update enemy animation
    updateEnemyAnimation(enemy);

    // Move the enemy
    moveEnemy(enemy, level, elapsed);

    // setting texture

    enemy.body.setTexture(enemy.anim[enemy.animationState].spritesheet);
    enemy.body.setTextureRect(enemy.anim[enemy.animationState].currentSprite);
}
void updateEnemyAnimation(Enemy& enemy)
{
    updateAnimation(enemy.anim[0]);

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
}//
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
        player.hurtSound.play();
    }

    if (enemy.body.getGlobalBounds().intersects(player.bullet.bullets_sprite.getGlobalBounds())) {
        enemy.isalive = 0;
        player.score += 5;
        player.bullet.together = 1;
        player.canThrow = true;
        player.bullet.soundPlay = true;
    }
}
void drawEnemy(Game& game) {
    if (game.map[game.currentMap].enemy.isalive == true) {
        game.window.draw(game.map[game.currentMap].enemy.body);
    }
}

void initbear(Bear& bear, Vector2f position) {
    bear.anim[0].spritesheet.loadFromFile("./assets/enemy/bear.png");
    initAnimation(bear.anim[0], 54, 63, 4);
    bear.anim[0].flipped = false; //sprite faces left, movement is right by default. 
    bear.velocity.x = 65;
    bear.health = 500;
    bear.body.setPosition(position);
    bear.body.setScale(Vector2f(2, 2));
}
void movebear(Bear& bear, Level level, float elapsed) {
    bear.velocity.y += GRAVITY * elapsed;
    if (bear.velocity.y > 500)
        bear.velocity.y = 500;
    bear.body.move(bear.velocity.x * elapsed, bear.velocity.y * elapsed);
    collisionX(bear, level);
    collisionY(bear, level);
}
void updateBear(Bear& bear, Level& level, float elapsed) {

    if (bear.jumpTimer.getElapsedTime().asSeconds() > 3.0f) {
        bear.velocity.y = -800;
        bear.jumpTimer.restart();
    }

    if (bear.health <= 0)
        bear.isalive = false;
    updatebearAnimation(bear);
    movebear(bear, level, elapsed);
    bear.body.setTexture(bear.anim[bear.animationState].spritesheet);
    bear.body.setTextureRect(bear.anim[bear.animationState].currentSprite);
}
void updatebearAnimation(Bear& bear) {
    updateAnimation(bear.anim[0]);
}
void collisionX(Bear& bear, Level& level) {
    int left_tile = bear.body.getPosition().x / TILE_WIDTH;
    int right_tile = left_tile + 3;
    int vertical_pos = bear.body.getPosition().y / TILE_WIDTH;
    //Sprite is approximately 3 tiles long (when rounded up) so we need to check the 3 tiles it covers horizontally
    //Since we're dealing with X collisions we don't need to check below it

    int moveDirection = sign(bear.velocity.x);
    for (int i = vertical_pos; i <= vertical_pos + 3; i++) {
        for (int j = left_tile; j <= right_tile; j++) {
            Tile& currentTile = level.tiles[j + i * level.width];

            if (currentTile.isSolid && bear.body.getGlobalBounds().intersects(currentTile.sprite.getGlobalBounds())) {
                bear.velocity.x *= -1;
                bear.anim[0].flipped = !bear.anim[0].flipped;
                bear.body.setPosition(currentTile.sprite.getPosition().x - (54 * 2 * moveDirection), bear.body.getPosition().y);
            }
        }
    }
}
void collisionY(Bear& bear, Level& level) {
    int above_tile = bear.body.getPosition().y / TILE_WIDTH;
    int below_tile = above_tile + 3;
    float horizontal_pos = bear.body.getPosition().x / TILE_WIDTH;

    if (bear.body.getPosition().y < 0)
        bear.body.setPosition(bear.body.getPosition().x, 0);

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
void CollisionPlayerWithEnemy(Bear& bear, Player& player)
{
    if (abs(player.velocity.x > 0) && player.hitbox.intersects(bear.body.getGlobalBounds())) {
        int moveDirection = sign(player.velocity.x);
        player.hitbox.left = bear.body.getPosition().x - 32 * moveDirection;
        player.velocity.x = 900 * -moveDirection;
        player.health--;
        player.hurtSound.play();
    }

    if (player.velocity.y > 0 && player.hitbox.intersects(bear.body.getGlobalBounds())) {
        player.hitbox.top = bear.body.getPosition().y - 32;
        player.velocity.y = -900;
        bear.health -= 100;
        player.score += 10;
    }

    if (bear.body.getGlobalBounds().intersects(player.bullet.bullets_sprite.getGlobalBounds())) {
        bear.health-= (100 * player.damageMultiplier);
        player.bullet.together = 1;
        player.canThrow = true;
        player.bullet.soundPlay = true;
    }
}
void drawBear(Game& game) {
    if (game.map[game.currentMap].bear.isalive == true) {
        game.window.draw(game.map[game.currentMap].bear.body);
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

void initLevel(Level& level,Font &font,int i) {
    level.spritesheet.loadFromFile("./assets/level/level_tileset.png");
    level.feather.loadFromFile("./assets/level/feather.png");
    level.backgroundImage.loadFromFile("./assets/level/back.png");
    level.txCoin.loadFromFile("./assets/level/gem.png");
    level.pipe.loadFromFile("./assets/level/pipe.png");
    level.spikeTexture.loadFromFile("assets/level/spike.png");
    level.flagTexture.loadFromFile("./assets/level/flag.png");
    level.boxTexture.loadFromFile("./assets/level/box.png");
    level.superTexture.loadFromFile("./assets/level/super_leaf.png");

    level.winBuffer.loadFromFile("./assets/audio/win.wav");//loading win buffer
    level.winSound.setBuffer(level.winBuffer);//setting buffer
    level.winSound.setVolume(50);
    level.playWinSound = true;


    loadLevelFile(level,i);

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
    level.win_text.setCharacterSize(96);
    //level.win_text.setFillColor(Color(170,100,255,255));
    level.win_text.setFillColor(Color(250, 200, 80, 255));
    level.win_text.setOutlineColor(Color::White);
    level.win_text.setOutlineThickness(2);

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
                level.tiles[j + i * level.width].sprite.setTextureRect(IntRect(64, 0, 32, 32));
                level.tiles[j + i * level.width].sprite.setScale(1, 1);
                level.tiles[j + i * level.width].isPipe = true;
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
            case '?':
                level.tiles[j + i * level.width].sprite.setTexture(level.boxTexture);
                level.tiles[j + i * level.width].sprite.setTextureRect(IntRect(0, 0, 16, 16));
                level.tiles[j + i * level.width].isBox = true;
                level.tiles[j + i * level.width].isSolid = true;
                break;
            case 'V':
                level.tiles[j + i * level.width].sprite.setTexture(level.superTexture);
                level.tiles[j + i * level.width].sprite.setTextureRect(IntRect(0, 0, 16, 16));
                level.tiles[j + i * level.width].isSuper = true;
                
            default: break;
            }


        }
    }
    if (i == 1) {
        initbear(level.bear,Vector2f(1600,0));
        level.bear.isalive = 0;
        initEnemy(level.enemy,Vector2f(55*32,14*32));
        level.enemy.isalive = 1;
    }
    else if (i == 2) {
        initbear(level.bear, Vector2f(3200, 0));
        level.bear.isalive = 1;
        initEnemy(level.enemy, Vector2f(32*42, 0));
        level.enemy.isalive = 1;
    }


    level.countdown_timer = 60;

}
void loadLevelFile(Level& level,int i) {
    std::string fileToLoad = "./assets/level/level" + std::to_string(i) + ".txt";
    std::ifstream levelFile(fileToLoad, std::ios::in);
    std::string temp;
    
    for (int i = 0; i < level.height && !levelFile.eof(); i++) {
        levelFile >> temp;
        level.map_string += temp;
    }
    levelFile.close();
}
void updateLevel(Game& game,float elapsed) {
    game.map[game.currentMap].score_text.setPosition(round(game.camera.getCenter().x-75), 0);
    game.map[game.currentMap].score_text.setString("SCORE\t  LIVES\n  " + std::to_string(game.player.score) + "\t\t\t" + std::to_string(game.player.lives));

    if (game.map[game.currentMap].timer.getElapsedTime().asSeconds() >= 1) {
        if (Win != true) // stopping timer
            game.map[game.currentMap].countdown_timer--;
        game.map[game.currentMap].timer_text.setString("TIME\n " + std::to_string(game.map[game.currentMap].countdown_timer));
        game.map[game.currentMap].timer.restart();
        if (game.map[game.currentMap].countdown_timer <= 0)
            gameover = 1;
    }

    game.map[game.currentMap].timer_text.setPosition(round(game.camera.getCenter().x + 200), 0);


    game.map[game.currentMap].background.setPosition(game.camera.getCenter());

    if (Win) {
        game.map[game.currentMap].score_text.setFillColor(Color::White);
        game.map[game.currentMap].timer_text.setFillColor(Color::White);
        game.map[game.currentMap].timer_text.setString("TIME\n " + std::to_string(LEVEL_TIMER - game.map[game.currentMap].countdown_timer));
        game.map[game.currentMap].win_text.setPosition(round(game.camera.getCenter().x) - 200, game.camera.getCenter().y - 150);
        game.map[game.currentMap].timer_text.setPosition(round(game.camera.getCenter().x + 50), game.camera.getCenter().y - 50);
        game.map[game.currentMap].score_text.setPosition(round(game.camera.getCenter().x) - 200, game.camera.getCenter().y - 50);
        if (game.currentMap == 0) {
            secondLevelUnlocked = true;
        }

        if (game.map[game.currentMap].playWinSound) {
            game.map[game.currentMap].winSound.play();//playimg sound 
            game.map[game.currentMap].playWinSound = false;
        }
    }

    updateBear(game.map[game.currentMap].bear, game.map[game.currentMap],elapsed);
    updateEnemy(game.map[game.currentMap].enemy, game.map[game.currentMap], elapsed);


}
void drawLevel(Game& game) {
    game.window.draw(game.map[game.currentMap].background);
    for (int i = 0; i < game.map[game.currentMap].height; i++) {
        for (int j = 0; j < game.map[game.currentMap].width; j++) {

            Tile &currentTile = game.map[game.currentMap].tiles[j + i * game.map[game.currentMap].width];
            if (currentTile.isSolid || currentTile.isPowerup || currentTile.isPipe || currentTile.isBox || currentTile.isSuper) { //make sure sprite isn't empty
                game.window.draw(game.map[game.currentMap].tiles[j + i * game.map[game.currentMap].width].sprite);
            }

            if (currentTile.isCoin) {
                game.window.draw(game.map[game.currentMap].tiles[j + i * game.map[game.currentMap].width].coin);
            }

            if (currentTile.isSpike){
                game.window.draw(game.map[game.currentMap].tiles[j + i * game.map[game.currentMap].width].spike);//drawing spike
            }

            if (currentTile.isFlag) {
                game.window.draw(game.map[game.currentMap].tiles[j + i * game.map[game.currentMap].width].flag);
            }
                

        }
    }
    game.window.draw(game.map[game.currentMap].timer_text);
    game.window.draw(game.map[game.currentMap].score_text);

    if (Win) {
        //Drawing win text alone in case of winning
        game.window.draw(game.map[game.currentMap].win_text);

    }

    
}