#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ncurses.h>
#include <unistd.h>
#include <locale.h>
#include <string.h>

#define MAP_WIDTH 100
#define MAP_HEIGHT 30
#define MAX_CORRIDOR_LENGTH 10
#define WALL_VERTICAL '|'
#define WALL_HORIZONTAL '_'
#define FLOOR_CHAR '.'
#define DOOR_CHAR '+'
#define CORRIDOR_CHAR '#'
#define OBJECT_CHAR 'O'
#define WALL_OBJECT_CHAR '='
#define LAST_ROOM_MARK '<'
#define HIDDEN_CHAR ' '
#define TALA '$' 
#define BLACK_GOLD_CHAR '&'
#define TRAP_HIDDEN_CHAR 'X' 
#define TRAP_DISCOVERED_CHAR '^'
#define SECRET_DOOR_HIDDEN '?'  
#define SECRET_DOOR_DISCOVERED '/' 
#define SPELL_HEALTH_CHAR ']'  
#define SPELL_SPEED_CHAR  '>'  
#define SPELL_DAMAGE_CHAR '!'
#define NORMAL_FOOD 'n'
#define SPELL_DURATION 10 
#define MAX_FOOD 5  
#define MAX_HUNGER 10  
#define MACE_CHAR 'g'      
#define DAGGER_CHAR 'k'    
#define MAGIC_WAND_CHAR 'a'
#define ARROW_CHAR 't'      
#define SWORD_CHAR 'w'      
#define NUM_WEAPONS (sizeof(available_weapons) / sizeof(Weapon))
#define COLOR_CUSTOM_PURPLE 7
#define COLOR_LIGHT_BLUE 8
#define COLOR_orange 9
#define DEMON_SYMBOL 'D'
#define FIRE_MONSTER_SYMBOL 'F'
#define GIANT_SYMBOL 'G'
#define SNAKE_SYMBOL 'S'
#define UNDEAD_SYMBOL 'U'
#define DEMON_HEALTH 5
#define FIRE_MONSTER_HEALTH 10
#define GIANT_HEALTH 15
#define SNAKE_HEALTH 20
#define UNDEAD_HEALTH 30
#define DEMON_DAMAGE 2
#define FIRE_MONSTER_DAMAGE 5
#define GIANT_DAMAGE 8
#define SNAKE_DAMAGE 10
#define UNDEAD_DAMAGE 15
#define GIANT_CHASE_STEPS 5
#define SNAKE_CHASE_STEPS -1  
#define UNDEAD_CHASE_STEPS 5


typedef struct {
    int x, y;
    int width, height;
} Room;


typedef struct {
    int x, y;                 
    int health;              
    int damage;              
    int chase_steps;           
    char symbol;               
    int is_alive;   
    Room room;          
} Enemy;

Enemy enemies[7] = {
        {0, 0, DEMON_HEALTH, DEMON_DAMAGE, DEMON_SYMBOL, 1},   
        {0, 0, FIRE_MONSTER_HEALTH, FIRE_MONSTER_DAMAGE, FIRE_MONSTER_SYMBOL, 1}, 
        {0, 0, GIANT_HEALTH, GIANT_DAMAGE, GIANT_SYMBOL, 1},  
        {0, 0, SNAKE_HEALTH, SNAKE_DAMAGE, SNAKE_SYMBOL, 1},  
        {0, 0, UNDEAD_HEALTH, UNDEAD_DAMAGE, UNDEAD_SYMBOL, 1}, 
    };
char weapon_symbols[] = {DAGGER_CHAR, MAGIC_WAND_CHAR, ARROW_CHAR, SWORD_CHAR};

typedef enum {
    MACE = 0,
    DAGGER,
    MAGIC_WAND,
    ARROW,
    SWORD
} WeaponType;

typedef struct {
    char symbol;
    char name[20];
    int damage;
    int range;
    int count;
} Weapon;


Weapon available_weapons[] = {
    {'g', "Mace", 5, 0, 1},
    {'k', "Dagger", 12, 5, 0},
    {'a', "Magic Wand", 15, 10, 0},
    {'t', "Arrow", 5, 5, 0},
    {'w', "Sword", 10, 0, 0}
};


Weapon current_weapon = {'g', "Mace", 5, 0, 1}; 


typedef struct {
    int normal_food_count;
    int hunger;
} Backpack_Food;

Backpack_Food player_backpack_food = {0,0};

// typedef struct {
//     FoodType type;
//     int health_restore; 
// } Food;

typedef struct {
    int spell_count;
    int active;       
    int remaining_moves; 
} SpellStatus;

typedef struct {
    SpellStatus health_spells;
    SpellStatus speed_spells;
    SpellStatus damage_spells;
} Backpack;

Backpack player_backpack = {
    {0, 0, 0},  
    {0, 0, 0},  
    {0, 0, 0}   
};


typedef struct {
    int x, y;
} Point;



typedef struct {
    int x, y;   
    int moves;
} Player;


char discovered_map[MAP_HEIGHT][MAP_WIDTH] = {0}; 
int tala=0;
int score=0;
int player_health = 100;
int player_damage_boost;

// void save_score() {
//     FILE *file = fopen("leaderboard.txt", "a");
//     if (file) {
//         fprintf(file, "Player: %s, Score: %d\n", loggedInUser.username, tala);
//         fclose(file);
//     }
// }

void handle_enemy_attack(Player *player, Enemy enemies[], int num_enemies) {
    for (int i = 0; i < num_enemies; i++) {
        if (enemies[i].is_alive) {
            int dx = abs(enemies[i].x - player->x);
            int dy = abs(enemies[i].y - player->y);

            if (dx <= 1 && dy <= 1) {
                player_health -= enemies[i].damage;
                mvprintw(0, 0, "Attacked by %c! Health: %d", enemies[i].symbol, player_health);
                refresh();
                sleep(1);
                mvprintw(0, 0, "                              ");
                refresh();

                if (player_health <= 0) {
                    mvprintw(MAP_HEIGHT + 2, 0, "Game Over! You died.");
                    refresh();
                    napms(2000);
                    endwin();
                    exit(0);
                }
            }
        }
    }
}


void move_enemy(Enemy *enemy, Player *player, char map[MAP_HEIGHT][MAP_WIDTH]) {
    if (!enemy->is_alive) return;

    if (enemy->chase_steps == 0) return;

    if (enemy->symbol == DEMON_SYMBOL || enemy->symbol == FIRE_MONSTER_SYMBOL) return;

    int dx = (player->x > enemy->x) ? 1 : (player->x < enemy->x) ? -1 : 0;
    int dy = (player->y > enemy->y) ? 1 : (player->y < enemy->y) ? -1 : 0;

    int new_x = enemy->x + dx;
    int new_y = enemy->y + dy;

    if (map[new_y][new_x] == FLOOR_CHAR) {
        enemy->x = new_x;
        enemy->y = new_y;
    }

    if (enemy->chase_steps > 0) {
        enemy->chase_steps--;
    }
}


int is_player_in_room(Player *player, Room room) {
    return (player->x >= room.x && player->x < room.x + room.width &&
            player->y >= room.y && player->y < room.y + room.height);
}


void place_random_enemies(char map[MAP_HEIGHT][MAP_WIDTH], Room rooms[], int num_rooms, int num_enemies) {
    char enemy_symbols[] = {DEMON_SYMBOL, FIRE_MONSTER_SYMBOL, GIANT_SYMBOL, SNAKE_SYMBOL, UNDEAD_SYMBOL};
    int health_values[] = {DEMON_HEALTH, FIRE_MONSTER_HEALTH, GIANT_HEALTH, SNAKE_HEALTH, UNDEAD_HEALTH};
    int damage_values[] = {DEMON_DAMAGE, FIRE_MONSTER_DAMAGE, GIANT_DAMAGE, SNAKE_DAMAGE, UNDEAD_DAMAGE};
    int chase_steps[] = {0, 0, GIANT_CHASE_STEPS, SNAKE_CHASE_STEPS, UNDEAD_CHASE_STEPS};

    int placed_enemies = 0;

    while (placed_enemies < num_enemies) {
        int room_index = rand() % num_rooms;
        Room selected_room = rooms[room_index];

        int x = selected_room.x + 1 + rand() % (selected_room.width - 2);
        int y = selected_room.y + 1 + rand() % (selected_room.height - 2);

        if (map[y][x] == FLOOR_CHAR) {  
            enemies[placed_enemies].x = x;
            enemies[placed_enemies].y = y;
            enemies[placed_enemies].symbol = enemy_symbols[placed_enemies % 5];
            enemies[placed_enemies].health = health_values[placed_enemies % 5];
            enemies[placed_enemies].damage = damage_values[placed_enemies % 5];
            enemies[placed_enemies].chase_steps = chase_steps[placed_enemies % 5];
            enemies[placed_enemies].is_alive = 1;
            enemies[placed_enemies].room = selected_room;  
            map[y][x] = enemies[placed_enemies].symbol;
            placed_enemies++;
        }
    }
}



void collect_weapon(char weapon_symbol) {
    for (int i = 0; i < 5; i++) {
        if (weapon_symbol == weapon_symbols[i]) {
            available_weapons[i+1].count++;
            mvprintw(0, 0, "You collected a weapon! Total %d of %c", available_weapons[i+1].count, weapon_symbols[i]);
            refresh();
            sleep(1);
            mvprintw(0, 0, "                                             ");
            refresh();
            break;
        }
    }
}


void place_random_weapons(char map[MAP_HEIGHT][MAP_WIDTH], Room rooms[], int room_count) {
    int weapon_count = rand() % 4;
    for (int i = 0; i < weapon_count; i++) {
        int room_index = rand() % room_count;
        Room room = rooms[room_index];
        int wx, wy;

        do {
            wx = room.x + 1 + rand() % (room.width - 2);
            wy = room.y + 1 + rand() % (room.height - 2);
        } while (map[wy][wx] != FLOOR_CHAR);

        WeaponType random_weapon = rand() % 4;
        map[wy][wx] = weapon_symbols[random_weapon];
    }
}


void collect_food(char food){
     player_backpack_food.normal_food_count++;
}

void collect_spell(char spell) {
    switch (spell) {
        case SPELL_HEALTH_CHAR:
            player_backpack.health_spells.spell_count++;
            break;
        case SPELL_SPEED_CHAR:
            player_backpack.speed_spells.spell_count++;
            break;
        case SPELL_DAMAGE_CHAR:
            player_backpack.damage_spells.spell_count++;
            break;
    }
}


void activate_spell(SpellStatus *spell) {
    spell->active = 1;
    spell->remaining_moves = 10;
}


void show_weapon_menu(){
    attron(COLOR_PAIR(2));
    mvprintw(MAP_HEIGHT + 2, 0, "=== Weapon Menu ===");
    attroff(COLOR_PAIR(2));
    mvprintw(MAP_HEIGHT + 3, 0, " Melee Weaspons:");
    mvprintw(MAP_HEIGHT + 4, 2, "Mace (g)-> count: %d , Damage: %d, Range: %d", available_weapons[0].count,available_weapons[0].damage,available_weapons[0].range);
    mvprintw(MAP_HEIGHT + 5, 2, "Sword (w)-> count: %d , Damage: %d, Range: %d", available_weapons[4].count,available_weapons[4].damage,available_weapons[4].range);
    mvprintw(MAP_HEIGHT + 6, 0, "Ranged Weaspon");
    mvprintw(MAP_HEIGHT + 7, 2, "Dagger(k)-> count: %d , Damage: %d, Range: %d", available_weapons[1].count,available_weapons[1].damage,available_weapons[1].range);
    mvprintw(MAP_HEIGHT + 8, 2, "Magic Wand(a)-> count: %d , Damage: %d, Range: %d", available_weapons[2].count,available_weapons[2].damage,available_weapons[2].range);
    mvprintw(MAP_HEIGHT + 9, 2, "Arrow (t)-> count: %d , Damage: %d, Range: %d", available_weapons[3].count,available_weapons[3].damage,available_weapons[3].range);
    refresh();

    sleep(5); // انتظار 2 ثانیه برای نمایش پیام‌ها
    mvprintw(MAP_HEIGHT + 2, 0, "                                                          "); // پاک کردن پیام با رشته خالی
    mvprintw(MAP_HEIGHT + 3, 0, "                                                          "); 
    mvprintw(MAP_HEIGHT + 4, 0, "                                                          ");
    mvprintw(MAP_HEIGHT + 5, 0, "                                                          ");
    mvprintw(MAP_HEIGHT + 6, 0, "                                                          ");
    mvprintw(MAP_HEIGHT + 7, 0, "                                                          ");
    mvprintw(MAP_HEIGHT + 8, 0, "                                                          ");
    mvprintw(MAP_HEIGHT + 9, 0, "                                                          ");
    refresh();

    int ch = getch();
    switch (ch) {
        case 'g':
        if(current_weapon.symbol == '@'){
            
               current_weapon.symbol = 'g';
                    strcpy(current_weapon.name, "Mace");
                    current_weapon.damage = 5;
                    current_weapon.range = 0;
                    current_weapon.count = 1;
                mvprintw(MAP_HEIGHT + 10, 0, "The default weapon changed to %s", current_weapon.name);
                refresh();
                sleep(2);
                mvprintw(MAP_HEIGHT + 10, 0, "                                                     "); 
                refresh();
        }
        else
        {
           mvprintw(MAP_HEIGHT + 10, 0,"First, put your weapon in the backpack.(p)");
                refresh();
                sleep(2);
                mvprintw(MAP_HEIGHT + 10, 0, "                                           "); 
                refresh();  
        }
        
        break;
        case 'w':
        if(current_weapon.symbol == '@'){
            if (available_weapons[4].count > 0) {
                available_weapons[4].count--;
               current_weapon.symbol = 'w';
                    strcpy(current_weapon.name, "Sword");
                    current_weapon.damage = 10;
                    current_weapon.range = 0;
                    current_weapon.count = 0;
                mvprintw(MAP_HEIGHT + 10, 0, "The default weapon changed to %s", current_weapon.name);
                refresh();
                sleep(2);
                mvprintw(MAP_HEIGHT + 10, 0, "                                                     "); 
                refresh();
            }
            else
            {
                mvprintw(MAP_HEIGHT + 10, 0,"This weapon is not available.");
                refresh();
                sleep(2);
                mvprintw(MAP_HEIGHT + 10, 0, "                                   "); 
                refresh();
            }
            
        }
        else
        {
           mvprintw(MAP_HEIGHT + 10, 0,"First, put your weapon in the backpack.(p)");
                refresh();
                sleep(2);
                mvprintw(MAP_HEIGHT + 10, 0, "                                           "); 
                refresh();  
        }
        
            break;
        case 'k':
        if(current_weapon.symbol == '@'){
            if (available_weapons[1].count > 0) {
                available_weapons[1].count--;
                current_weapon.symbol = 'k';
                    strcpy(current_weapon.name, "Dagger");
                    current_weapon.damage = 12;
                    current_weapon.range = 5;
                    current_weapon.count = 0;
                mvprintw(MAP_HEIGHT + 10, 0, "The default weapon changed to %s", current_weapon.name);
                refresh();
                sleep(2);
                mvprintw(MAP_HEIGHT + 10, 0, "                                                    "); 
                refresh();
            }
            else
            {
                mvprintw(MAP_HEIGHT + 10, 0,"This weapon is not available.");
                refresh();
                sleep(2);
                mvprintw(MAP_HEIGHT + 10, 0, "                                   "); 
                refresh();
            }
            
        }
        else
        {
           mvprintw(MAP_HEIGHT + 10, 0,"First, put your weapon in the backpack.(p)");
                refresh();
                sleep(2);
                mvprintw(MAP_HEIGHT + 10, 0, "                                           "); 
                refresh();  
        }
        
            break;
        case 't':
        if(current_weapon.symbol == '@'){
            if (available_weapons[3].count > 0) {
                available_weapons[3].count--;
                current_weapon.symbol = 't';
                    strcpy(current_weapon.name, "Arrow");
                    current_weapon.damage = 5;
                    current_weapon.range = 5;
                    current_weapon.count = 0;
                mvprintw(MAP_HEIGHT + 10, 0, "The default weapon changed to %s", current_weapon.name);
                refresh();
                sleep(2);
                mvprintw(MAP_HEIGHT + 10, 0, "                                                   "); 
                refresh();
            }
            else
            {
                mvprintw(MAP_HEIGHT + 10, 0,"This weapon is not available.");
                refresh();
                sleep(2);
                mvprintw(MAP_HEIGHT + 10, 0, "                                                    "); 
                refresh();
            }
            
        }
        else
        {
           mvprintw(MAP_HEIGHT + 10, 0,"First, put your weapon in the backpack.(p)");
                refresh();
                sleep(2); 
                mvprintw(MAP_HEIGHT + 10, 0, "                                                   "); 
                refresh();  
        }
            break;
        case 'a':
        if(current_weapon.symbol == '@'){
            if (available_weapons[2].count > 0) {
                available_weapons[2].count--;
                current_weapon.symbol = 'a';
                    strcpy(current_weapon.name, "Magic Wand");
                    current_weapon.damage = 15;
                    current_weapon.range = 10;
                    current_weapon.count = 0;
                mvprintw(MAP_HEIGHT + 10, 0, "The default weapon changed to %s", current_weapon.name);
                refresh();
                sleep(2);
                mvprintw(MAP_HEIGHT + 10, 0, "                                                              "); 
                refresh();
            }
            else
            {
                mvprintw(MAP_HEIGHT + 10, 0,"This weapon is not available.");
                refresh();
                sleep(2);
                mvprintw(MAP_HEIGHT + 10, 0, "                                   "); 
                refresh();
            }
            
        }
        else
        {
           mvprintw(MAP_HEIGHT + 10, 0,"First, put your weapon in the backpack.(p)");
                refresh();
                sleep(2);
                mvprintw(MAP_HEIGHT + 10, 0, "                                           "); 
                refresh();  
        }
            break;
    }
}


void show_backpack_food() {
    mvprintw(MAP_HEIGHT + 2, 0, "Backpack_Food:");
    mvprintw(MAP_HEIGHT + 3, 0, "Normal Food (n): %d", player_backpack_food.normal_food_count);
    mvprintw(MAP_HEIGHT + 4, 0, "Hunger: %d/%d", player_backpack_food.hunger, MAX_HUNGER);
    mvprintw(MAP_HEIGHT + 5, 0, "Health: %d", player_health);
    mvprintw(MAP_HEIGHT + 6, 0, "Hunger: [");
    for (int i = 0; i < player_backpack_food.hunger; i++) {
            mvprintw(MAP_HEIGHT + 6, 10 + i, "*");  
    }

        mvprintw(MAP_HEIGHT + 6, 20 +player_backpack_food.hunger, "]");  
        refresh();
        sleep(2);
        mvprintw(MAP_HEIGHT + 2, 0, "                                "); 
        mvprintw(MAP_HEIGHT + 3, 0, "                                "); 
        mvprintw(MAP_HEIGHT + 4, 0, "                                ");
        mvprintw(MAP_HEIGHT + 5, 0, "                                ");
        mvprintw(MAP_HEIGHT + 6, 0, "                                                                 ");
       
    refresh();


    int ch = getch();
    if (ch == 'n' ) {
        player_health+=5;
        player_backpack_food.normal_food_count--;
        if (player_backpack_food.hunger > 0) player_backpack_food.hunger -= 5;  
        if (player_backpack_food.hunger <= 0) player_backpack_food.hunger = 0;}
        mvprintw(MAP_HEIGHT + 7, 0, "Health increased! Health: %d", player_health);
                refresh();
                sleep(2);
                mvprintw(MAP_HEIGHT + 7, 0, "                                "); 
                refresh();
}

void show_backpack() {
    mvprintw(MAP_HEIGHT + 2, 0, "Backpack:");
    mvprintw(MAP_HEIGHT + 3, 0, "Health Spells (]): %d", player_backpack.health_spells.spell_count);
    mvprintw(MAP_HEIGHT + 4, 0, "Speed Spells (>): %d", player_backpack.speed_spells.spell_count);
    mvprintw(MAP_HEIGHT + 5, 0, "Damage Spells (!): %d", player_backpack.damage_spells.spell_count);
    refresh();

    sleep(2);
    mvprintw(MAP_HEIGHT + 2, 0, "                                ");
    mvprintw(MAP_HEIGHT + 3, 0, "                                "); 
    mvprintw(MAP_HEIGHT + 4, 0, "                                ");
    mvprintw(MAP_HEIGHT + 5, 0, "                                ");
    refresh();

    int ch = getch();
    switch (ch) {
        case ']':
            if (player_backpack.health_spells.spell_count > 0) {
                player_backpack.health_spells.spell_count--;
                if (player_health <= 80)
                {
                    player_health += 20; 
                }
                else
                {
                    player_health = 100;
                }
                mvprintw(MAP_HEIGHT + 6, 0, "Health spell used! Health: %d", player_health);
                refresh();
                sleep(2);
                mvprintw(MAP_HEIGHT + 6, 0, "                                "); 
                refresh();
            }
            break;
        case '>':
            if (player_backpack.speed_spells.spell_count > 0) {
                player_backpack.speed_spells.spell_count--;
                activate_spell(&player_backpack.speed_spells);
                mvprintw(MAP_HEIGHT + 6, 0, "Speed spell used!");
                refresh();
                sleep(2);
                mvprintw(MAP_HEIGHT + 6, 0, "                                "); 
                refresh();
            }
            break;
        case '!':
            if (player_backpack.damage_spells.spell_count > 0) {
                player_backpack.damage_spells.spell_count--;
                player_damage_boost = SPELL_DURATION;
                mvprintw(MAP_HEIGHT + 6, 0, "Damage spell used!");
                refresh();
                sleep(2);
                mvprintw(MAP_HEIGHT + 6, 0, "                                "); 
                refresh();
            }
            break;
    }
}

void place_random_magic_items(char map[MAP_HEIGHT][MAP_WIDTH], Room rooms[], int num_rooms) {
    int item_count = 3; 
    char magic_symbols[3] = {SPELL_HEALTH_CHAR,SPELL_SPEED_CHAR,SPELL_DAMAGE_CHAR };

    srand(time(NULL));// سعی کن فقط یبار srand بزنی

    for (int i = 0; i < item_count; i++) {
        int placed = 0;

        while (!placed) {
            int room_index = rand() % num_rooms;
            Room selected_room = rooms[room_index];

            int x = selected_room.x + 1 + rand() % (selected_room.width - 2);
            int y = selected_room.y + 1 + rand() % (selected_room.height - 2);

            if (map[y][x] == FLOOR_CHAR) { 
                map[y][x] = magic_symbols[i]; 
                placed = 1;
            }
        }
    }
}

void place_secret_door(char map[MAP_HEIGHT][MAP_WIDTH], Room room[]) {
    int sx, sy;
    int placed = 0;

    while (!placed) {
        if (rand() % 2) {
            sy = (rand() % 2) ? room->y : room->y + room->height - 1;
            sx = room->x + 1 + rand() % (room->width - 2);
        } else {
            sx = (rand() % 2) ? room->x : room->x + room->width - 1;
            sy = room->y + 1 + rand() % (room->height - 2);
        }

        if (map[sy][sx] == WALL_HORIZONTAL || map[sy][sx] == WALL_VERTICAL) {
            map[sy][sx] = SECRET_DOOR_HIDDEN;
            placed = 1;
        }
    }
     refresh();
}


void discover_secret_door(char map[MAP_HEIGHT][MAP_WIDTH], Player *player) {
    int dx[] = {1, -1, 0, 0};
    int dy[] = {0, 0, 1, -1};

    for (int i = 0; i < 4; i++) {
        int nx = player->x + dx[i];
        int ny = player->y + dy[i];

        if (nx >= 0 && ny >= 0 && nx < MAP_WIDTH && ny < MAP_HEIGHT) {
            if (map[ny][nx] == SECRET_DOOR_HIDDEN) {
                map[ny][nx] = SECRET_DOOR_DISCOVERED;
            }
        }
    }
}


void place_treasure_room(char map[MAP_HEIGHT][MAP_WIDTH], Room rooms[], int room_count) {
    Room treasure_room;
    int placed = 0;

    
    while (!placed) {
        int room_index = rand() % room_count;
        treasure_room = rooms[room_index];

            placed = 1;
        
    }

   
    for (int y = treasure_room.y; y < treasure_room.y + treasure_room.height; y++) {
        for (int x = treasure_room.x; x < treasure_room.x + treasure_room.width; x++) {
            if (map[y][x] == FLOOR_CHAR) {
                map[y][x] = '*'; 
            }
        }
    }

   
    int trap_count = 5 + rand() % 3;  
    for (int i = 0; i < trap_count; i++) {
        int tx, ty;
        do {
            tx = treasure_room.x + 1 + rand() % (treasure_room.width - 2);
            ty = treasure_room.y + 1 + rand() % (treasure_room.height - 2);
        } while (map[ty][tx] != '*');
        map[ty][tx] = TRAP_HIDDEN_CHAR;
    }
}

void place_food_in_rooms(char map[MAP_HEIGHT][MAP_WIDTH], Room rooms[], int num_rooms) {
    int item_count = 5; 

    for (int i = 0; i < item_count; i++) {
        int placed = 0;

        while (!placed) {
            int room_index = rand() % num_rooms;
            Room selected_room = rooms[room_index];

            int x = selected_room.x + 1 + rand() % (selected_room.width - 2);
            int y = selected_room.y + 1 + rand() % (selected_room.height - 2);

            if (map[y][x] == FLOOR_CHAR) { 
                map[y][x] =NORMAL_FOOD; 
                placed = 1;
            }
        }
    }
}

void place_traps_in_rooms(char map[MAP_HEIGHT][MAP_WIDTH], Room rooms[], int num_rooms) {
    int selected_rooms[3] = {-1, -1, -1};  // ذخیره اندیس اتاق‌هایی که تله دارند
    int trap_rooms_count = 0;


    while (trap_rooms_count < 3 && trap_rooms_count < num_rooms) {
        int room_index = rand() % num_rooms;

        // بررسی اینکه این اتاق قبلاً انتخاب نشده باشد
        int already_selected = 0;
        for (int i = 0; i < trap_rooms_count; i++) {
            if (selected_rooms[i] == room_index) {
                already_selected = 1;
                break;
            }
        }
        if (already_selected) continue;

        // انتخاب اتاق و گذاشتن تله
        selected_rooms[trap_rooms_count++] = room_index;
        Room selected_room = rooms[room_index];

        int trap_count = rand() % 2;  // تعداد تله‌های داخل اتاق (1 یا 2)
        for (int i = 0; i < trap_count; i++) {
            int tx, ty;
            do {
                tx = selected_room.x + 1 + rand() % (selected_room.width - 2);
                ty = selected_room.y + 1 + rand() % (selected_room.height - 2);
            } while (map[ty][tx] != FLOOR_CHAR);
            map[ty][tx] = TRAP_HIDDEN_CHAR;  // تله مخفی
        }
    }
}


void place_black_gold(char map[MAP_HEIGHT][MAP_WIDTH], Room rooms[], int num_rooms)
 {
    int room_index = rand() % num_rooms;
    Room selected_room = rooms[room_index];

    int x, y;
    int placed = 0;
    
    while (!placed) {
        
        x = selected_room.x + 1 + rand() % (selected_room.width - 2);
        y = selected_room.y + 1 + rand() % (selected_room.height - 2);

      
        if (map[y][x] == FLOOR_CHAR) {
           
            map[y][x] = BLACK_GOLD_CHAR;
            placed = 1;  
        }
    }
}

void draw_map_with_visibility(char map[MAP_HEIGHT][MAP_WIDTH], char visible_map[MAP_HEIGHT][MAP_WIDTH]) {
    for (int i = 0; i < MAP_HEIGHT; i++) {
        for (int j = 0; j < MAP_WIDTH; j++) {
            if (visible_map[i][j]) {
                if (map[i][j] == WALL_HORIZONTAL || map[i][j] == WALL_VERTICAL || map[i][j] == SECRET_DOOR_HIDDEN) {
                    attron(COLOR_PAIR(1));
                } else if (map[i][j] == CORRIDOR_CHAR || map[i][j] == BLACK_GOLD_CHAR) {
                    attron(COLOR_PAIR(2));
                } else if (map[i][j] == FLOOR_CHAR || map[i][j] == TRAP_HIDDEN_CHAR) {
                    attron(COLOR_PAIR(3));
                } else if (map[i][j] == OBJECT_CHAR || map[i][j] == WALL_OBJECT_CHAR || map[i][j] == TALA ) {
                    attron(COLOR_PAIR(4));
                }
                else if (map[i][j] == TRAP_DISCOVERED_CHAR || map[i][j] == SECRET_DOOR_DISCOVERED) {
                    attron(COLOR_PAIR(5));  
                }
                else if (map[i][j] == SPELL_HEALTH_CHAR || map[i][j] == SPELL_SPEED_CHAR || map[i][j] == SPELL_DAMAGE_CHAR)
                {
                    attron(COLOR_PAIR(6));
                }
                else if (map[i][j] == DAGGER_CHAR || map[i][j] == MAGIC_WAND_CHAR|| map[i][j] == ARROW_CHAR|| map[i][j] == SWORD_CHAR || map[i][j] == MACE_CHAR)
                {
                     attron(COLOR_PAIR(8));
                }
                 else if (map[i][j] == SNAKE_SYMBOL || map[i][j] == DEMON_SYMBOL|| map[i][j] == UNDEAD_SYMBOL|| map[i][j] == GIANT_SYMBOL || map[i][j] ==FIRE_MONSTER_SYMBOL )
                {
                     attron(COLOR_PAIR(9));
                }
                
                mvaddch(i, j, map[i][j]);
                
                attroff(COLOR_PAIR(1) | COLOR_PAIR(2) | COLOR_PAIR(3) | COLOR_PAIR(4) | COLOR_PAIR(5) | COLOR_PAIR(6) | COLOR_PAIR(8) | COLOR_PAIR(9));
            } else {
                mvaddch(i, j, HIDDEN_CHAR);
            }
        }
    }
}


void reveal_area(char visible_map[MAP_HEIGHT][MAP_WIDTH],int x, int y, int radius) {
    for (int dy = -radius; dy <= radius; dy++) {
        for (int dx = -radius; dx <= radius; dx++) {
            int nx = x + dx;
            int ny = y + dy;
            if (nx >= 0 && ny >= 0 && nx < MAP_WIDTH && ny < MAP_HEIGHT) {
                visible_map[ny][nx] = 1;
                discovered_map[ny][nx] = 1; 
            }
        }
    }
}


void place_player(Room first_room, Player *player) {
    player->x = first_room.x + first_room.width / 2;
    player->y = first_room.y + first_room.height / 2;
    mvaddch(player->y, player->x, current_weapon.symbol); 
}

void move_player_with_visibility(char map[MAP_HEIGHT][MAP_WIDTH],char visible_map[MAP_HEIGHT][MAP_WIDTH], Player *player, int dx, int dy) {
    int step_size = ( player_backpack.speed_spells.active > 0) ? 2 : 1; 
    int new_x = player->x + dx * step_size;
    int new_y = player->y + dy * step_size;

    player->moves+=step_size; 

    if (player->moves % 20 == 0) { 
        player_backpack_food.hunger++;
        if (player_backpack_food.hunger > MAX_HUNGER) 
        {player_backpack_food.hunger = MAX_HUNGER;
         player_health--;
        }


        mvprintw(0, 0, "Hunger increased! Current Hunger: %d", player_backpack_food.hunger);
        if (player_backpack_food.hunger == MAX_HUNGER){
             mvprintw(1, 0, "Health dicreased! Currnt Health: %d", player_health);
        }
        refresh();
        sleep(1);
        mvprintw(0, 0, "                                    ");
        mvprintw(1, 0, "                                    ");
        refresh();
    }

    if (new_x >= 0 && new_x < MAP_WIDTH && new_y >= 0 && new_y < MAP_HEIGHT) {
        char target = map[new_y][new_x];
        
        if (target == FLOOR_CHAR || target == CORRIDOR_CHAR || 
            (target == DOOR_CHAR && map[player->y][player->x] != WALL_VERTICAL && map[player->y][player->x] != WALL_HORIZONTAL) &&
            target != OBJECT_CHAR && target != ' ' && target != WALL_OBJECT_CHAR || 
            target == LAST_ROOM_MARK || target == TALA || target == BLACK_GOLD_CHAR || 
            target == TRAP_HIDDEN_CHAR || target == '*' || target == SECRET_DOOR_DISCOVERED || 
            target == SPELL_DAMAGE_CHAR || target == SPELL_HEALTH_CHAR || target == SPELL_SPEED_CHAR||
            target == NORMAL_FOOD ||target== DAGGER_CHAR || target == MAGIC_WAND_CHAR ||target== ARROW_CHAR || target== SWORD_CHAR ) {
            
            if (target== DAGGER_CHAR || target == MAGIC_WAND_CHAR ||target== ARROW_CHAR || target== SWORD_CHAR )
            {
                map[new_y][new_x] = FLOOR_CHAR;
                collect_weapon(target);
            }
            
            if (target == NORMAL_FOOD)
            {
                map[new_y][new_x] = FLOOR_CHAR;
                collect_food(target);
            }
            
            if (target == SPELL_SPEED_CHAR) {
                map[new_y][new_x] = FLOOR_CHAR;
                collect_spell(target);
            }
            if (target == TALA || target == BLACK_GOLD_CHAR) {
                map[new_y][new_x] = FLOOR_CHAR;
            } else if (target == TRAP_HIDDEN_CHAR) {
                map[new_y][new_x] = TRAP_DISCOVERED_CHAR;
            }
            else if (target==SPELL_DAMAGE_CHAR || target==SPELL_HEALTH_CHAR )
                {
                    map[new_y][new_x] = FLOOR_CHAR;
                    collect_spell(target);
                }

            mvaddch(player->y, player->x, map[player->y][player->x]);

            player->x = new_x;
            player->y = new_y;
            
            for (int i = 0; i < 5; i++) {
            if (enemies[i].is_alive && is_player_in_room(player, enemies[i].room)) {
                move_enemy(&enemies[i], player, map);
            }
            }
         
           if (player_backpack.speed_spells.active) {
            player_backpack.speed_spells.remaining_moves--;
            if (player_backpack.speed_spells.remaining_moves <= 0) {
                player_backpack.speed_spells.active = 0;
                mvprintw(0, 0, "Speed spell effect expired!");
                refresh();
                napms(1000);
            }
        }


            discover_secret_door(map, player);
            reveal_area(visible_map, player->x, player->y, 5);
            draw_map_with_visibility(map, visible_map);
            mvaddch(player->y, player->x, current_weapon.symbol);
            refresh();
             if (target == TALA) {
                tala += 5;
                mvprintw(0, 0, "You collected $! Total Tala: %d", tala); 
                refresh();
                sleep(1); 
                mvprintw(0, 0, "                                 "); 
                refresh();
            }
            if (target == BLACK_GOLD_CHAR) {
                tala += 20;
                mvprintw(0, 0, "You collected &! Total Tala: %d", tala);
                refresh();
                sleep(2); 
                mvprintw(0, 0, "                                 "); 
                refresh();
            }
            if (target == TRAP_HIDDEN_CHAR) {
                    player_health -= 10;
                    mvprintw(0, 0, "Trap discovered! Health -10. You'r Health is %d", player_health); 
                    refresh();
                    sleep(1); 
                    mvprintw(0, 0, "                                                     "); 
                    refresh();
                }
                if (player_health <= 0) {
                    clear();
                    refresh();
                    mvprintw(15, 50, "Game Over! You died.");
                    refresh();
                    napms(2000);
                    endwin();
                    exit(0);
                }
        }
    }
}

void handle_visibility_toggle(char map[MAP_HEIGHT][MAP_WIDTH],char visible_map[MAP_HEIGHT][MAP_WIDTH],int *fullMapReveal) {
    if (*fullMapReveal) {
      
        for (int i = 0; i < MAP_HEIGHT; i++) {
            for (int j = 0; j < MAP_WIDTH; j++) {
                visible_map[i][j] = discovered_map[i][j];
            }
        }
        *fullMapReveal = 0;
    } else {
       
        for (int i = 0; i < MAP_HEIGHT; i++) {
            for (int j = 0; j < MAP_WIDTH; j++) {
                visible_map[i][j] = 1;
            }
        }
        *fullMapReveal = 1;
    }
    draw_map_with_visibility(map, visible_map);
}


void handle_player_movement(char map[MAP_HEIGHT][MAP_WIDTH],char visible_map[MAP_HEIGHT][MAP_WIDTH], Player *player, int *fullMapReveal) {
    int key;
    keypad(stdscr, TRUE);

    while (1) {
        key = getch();

        if (key == 'M' || key == 'm') {
            handle_visibility_toggle(map, visible_map,fullMapReveal);
            mvaddch(player->y, player->x,current_weapon.symbol);
            refresh();
        }
        else if (key == 'p' || key == 'P')
        {
            current_weapon.symbol='@';
            strcpy(current_weapon.name, "Player");
            current_weapon.damage = 0;
            current_weapon.range = 0;
            current_weapon.count = 1;
        }
        
        else if (key == 'b' || key == 'B')
        {
            show_backpack();
            refresh();
            
        }
        else if (key == 'E' || key == 'e')
        {
            show_backpack_food();
            refresh();
        }
        else if (key == 'i' || key == 'I') {
            show_weapon_menu();
            refresh();
        }
        

        if (map[player->y][player->x] == LAST_ROOM_MARK) {
            break;
        }
        if (map[player->y][player->x] == '*') {
            mvprintw(MAP_HEIGHT + 2, 0, "You found the treasure room! Game Over.");
            // save_score();  //ذخیره امتیاز
            refresh();
            napms(3000);
            endwin();
            exit(0);
        }


        switch (key) {
            case '7': move_player_with_visibility(map, visible_map, player, -1, -1); break;
            case '8': move_player_with_visibility(map, visible_map, player, 0, -1); break;
            case '9': move_player_with_visibility(map, visible_map, player, 1, -1); break;
            case '4': move_player_with_visibility(map, visible_map, player, -1, 0); break;
            case '6': move_player_with_visibility(map, visible_map, player, 1, 0); break;
            case '1': move_player_with_visibility(map, visible_map, player, -1, 1); break;
            case '2': move_player_with_visibility(map, visible_map, player, 0, 1); break;
            case '3': move_player_with_visibility(map, visible_map, player, 1, 1); break;
        }
        
    

        refresh();

        handle_enemy_attack(player, enemies, 5);
    }
}

void initialize_map(char map[MAP_HEIGHT][MAP_WIDTH]) {
    for (int i = 0; i < MAP_HEIGHT; i++) {
        for (int j = 0; j < MAP_WIDTH; j++) {
            map[i][j] = ' ';
        }
    }
}

int bfs_find_path(char map[MAP_HEIGHT][MAP_WIDTH], int x1, int y1, int x2, int y2) {
    int dx[] = {1, -1, 0, 0};
    int dy[] = {0, 0, 1, -1};
    char visited[MAP_HEIGHT][MAP_WIDTH] = {0};
    Point parent[MAP_HEIGHT][MAP_WIDTH];
    Point queue[MAP_WIDTH * MAP_HEIGHT];
    int front = 0, rear = 0;

    queue[rear++] = (Point){x1, y1};
    visited[y1][x1] = 1;

    while (front < rear) {
        Point current = queue[front++];

        if (current.x == x2 && current.y == y2) {
           
            Point p = current;
            map[y1][x1] = DOOR_CHAR;
            map[y2][x2] = DOOR_CHAR;
            while (!(p.x == x1 && p.y == y1)) {
                if (map[p.y][p.x] != DOOR_CHAR) { 
                    map[p.y][p.x] = CORRIDOR_CHAR;
                }
                p = parent[p.y][p.x];
            }
            return 1; 
        }

        for (int i = 0; i < 4; i++) {
            int nx = current.x + dx[i];
            int ny = current.y + dy[i];

            if (nx >= 0 && ny >= 0 && nx < MAP_WIDTH && ny < MAP_HEIGHT &&
                !visited[ny][nx] && (map[ny][nx] == ' ' || map[ny][nx] == DOOR_CHAR)) {
                queue[rear++] = (Point){nx, ny};
                visited[ny][nx] = 1;
                parent[ny][nx] = current;
            }
        }
    }
    return 0; 
}

int place_room(char map[MAP_HEIGHT][MAP_WIDTH], Room room) {
    for (int i = room.y; i < room.y + room.height; i++) {
        for (int j = room.x; j < room.x + room.width; j++) {
            if (map[i][j] != ' ') {
                return 0; 
            }
        }
    }

    for (int i = room.y; i < room.y + room.height; i++) {
        for (int j = room.x; j < room.x + room.width; j++) {
            if (i == room.y || i == room.y + room.height - 1)
                map[i][j] = WALL_HORIZONTAL;
            else if (j == room.x || j == room.x + room.width - 1)
                map[i][j] = WALL_VERTICAL;
            else
                map[i][j] = FLOOR_CHAR;
        }
    }

    return 1;
}

int compareRooms(const void *a, const void *b) {
    Room *roomA = (Room *)a;
    Room *roomB = (Room *)b;
    return (roomA->x + roomA->width / 2) - (roomB->x + roomB->width / 2);
}

void place_random_objects(char map[MAP_HEIGHT][MAP_WIDTH], Room room) {
    int object_count = rand() % 2; 
    for (int i = 0; i < object_count; i++) {
        int ox, oy;
        do {
            ox = room.x + 1 + rand() % (room.width - 2);
            oy = room.y + 1 + rand() % (room.height - 2);
        } while (map[oy][ox] != FLOOR_CHAR);
        map[oy][ox] = OBJECT_CHAR;
    }

    int tala_count = rand() % 3; 
   for (int i = 0; i < tala_count; i++) {
    int tx, ty;
    do {
        tx = room.x + 1 + rand() % (room.width - 2);
        ty = room.y + 1 + rand() % (room.height - 2);
    } while (map[ty][tx] != FLOOR_CHAR);
    map[ty][tx] = TALA;
    }


    // قرار دادن علامت '=' روی دیوارها بدون قرارگیری روی درها
    int wall_object_count = rand() % 3;
    for (int i = 0; i < wall_object_count; i++) {
        int wx, wy;
        do {
            if (rand() % 2) { // دیوار افقی
                wy = (rand() % 2) ? room.y : room.y + room.height - 1;
                wx = room.x + 1 + rand() % (room.width - 2);
            } else { // دیوار عمودی
                wx = (rand() % 2) ? room.x : room.x + room.width - 1;
                wy = room.y + 1 + rand() % (room.height - 2);
            }
        } while (map[wy][wx] != WALL_HORIZONTAL && map[wy][wx] != WALL_VERTICAL);
        map[wy][wx] = WALL_OBJECT_CHAR;
    }
}
//o
int is_adjacent_to_object(char map[MAP_HEIGHT][MAP_WIDTH], int x, int y) {
    int dx[] = {1, -1, 0, 0};
    int dy[] = {0, 0, 1, -1};

    for (int i = 0; i < 4; i++) {
        int nx = x + dx[i];
        int ny = y + dy[i];

        if (nx >= 0 && ny >= 0 && nx < MAP_WIDTH && ny < MAP_HEIGHT) {
            if (map[ny][nx] == OBJECT_CHAR) {
                return 1; 
            }
        }
    }
    return 0; 
}

void connect_rooms(char map[MAP_HEIGHT][MAP_WIDTH], Room r1, Room r2, int connect_right_to_left) {
    int x1, y1, x2, y2;

    do {
        if (connect_right_to_left == 1) {
        y1 = r1.y + 1 + rand() % (r1.height - 2); 
        x1 = r1.x + r1.width - 1;
        y2 = r2.y + 1 + rand() % (r2.height - 2); 
        x2 = r2.x;
    }
    
    else if(connect_right_to_left == 2) {
        y1 = r1.y + 1 + rand() % (r1.height - 2);
        x1 = r1.x + r1.width - 1;
        y2 = r2.y + 1 + rand() % (r2.height - 2); 
        x2 = r2.x + r2.width - 1;
    }
     else {
        y1 = r1.y + 1 + rand() % (r1.height - 2); 
        x1 = r1.x;
        y2 = r2.y + 1 + rand() % (r2.height - 2); 
        x2 = r2.x + r2.width - 1;
    }
    } while (is_adjacent_to_object(map, x1, y1) || is_adjacent_to_object(map, x2, y2));

    map[y1][x1] = DOOR_CHAR;
    map[y2][x2] = DOOR_CHAR;

    if (!bfs_find_path(map, x1, y1, x2, y2)) {
        printf("No path found between (%d, %d) and (%d, %d)\n", x1, y1, x2, y2);
    }
}

//otagh
void generate_random_map(char map[MAP_HEIGHT][MAP_WIDTH],Room rooms[], int room_count) {
   
    int placed_rooms = 0;

    while (placed_rooms < room_count) {
        Room new_room;
        new_room.width = 6 + rand() % 5;
        new_room.height = 6 + rand() % 5;

        switch (placed_rooms) {
            case 0:
                new_room.x = 2 + rand() % (30 - new_room.width - 2);
                new_room.y = 2 + rand() % (14 - new_room.height - 2);
                break;
            case 1: 
                new_room.x = 32 + rand() % (60 - new_room.width - 32);
                new_room.y = 2 + rand() % (14 - new_room.height - 2);
                break;
            case 2:
                new_room.x = 62 + rand() % (98 - new_room.width - 62);
                new_room.y = 2 + rand() % (14 - new_room.height - 2);
                break;
            case 3: 
                new_room.x = 62 + rand() % (98 - new_room.width - 62);
                new_room.y = 16 + rand() % (30 - new_room.height - 16);
                break;
            case 4: 
                new_room.x = 32 + rand() % (60 - new_room.width - 32);
                new_room.y = 16 + rand() % (30 - new_room.height - 16);
                break;
            case 5: 
                new_room.x = 2 + rand() % (30 - new_room.width - 2);
                new_room.y = 16 + rand() % (30 - new_room.height - 16);
                break;
        }

        if (place_room(map, new_room)) {
    place_random_objects(map, new_room); 
    rooms[placed_rooms++] = new_room;

    //اضافه کردن علامت > ر
    if (placed_rooms == room_count) { 
        int lx, ly;
        do {
            lx = new_room.x; 
            ly = new_room.y + 1 + rand() % (new_room.height - 2);
        } while (map[ly][lx] != WALL_VERTICAL);

        map[ly][lx] = LAST_ROOM_MARK;
    }
    
}


    }

     for (int i = 0; i < room_count - 4; i++) {
        connect_rooms(map, rooms[i], rooms[i + 1],1);
    }
    
    for (int i = 2; i < room_count - 3; i++) {
        connect_rooms(map, rooms[i], rooms[i + 1],2);
    }
    for (int i = 3; i < room_count - 1; i++) {
        connect_rooms(map, rooms[i], rooms[i + 1],0);
    }
}


int main() {
    setlocale(LC_ALL, "");
   
    initscr();
    start_color();
    if (can_change_color()) {
        init_color(COLOR_orange, 1000, 500, 0);
    }
    if (can_change_color()) {
        init_color(COLOR_MAGENTA, 1000, 500, 800);
    }
    if (has_colors() && can_change_color()) {
        init_color(COLOR_CUSTOM_PURPLE, 700, 300, 900); 
    }
    if (has_colors() && can_change_color()) {
    init_color(COLOR_LIGHT_BLUE, 500, 500, 1000);
    }
   
    init_pair(1, COLOR_RED, COLOR_BLACK); 
    init_pair(2, COLOR_BLUE, COLOR_BLACK);
    init_pair(3, COLOR_GREEN, COLOR_BLACK); 
    init_pair(4, COLOR_YELLOW, COLOR_BLACK); 
    init_pair(5, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(6, COLOR_MAGENTA, COLOR_BLACK); 
    init_pair(7, COLOR_CUSTOM_PURPLE, COLOR_BLACK);
    init_pair(8, COLOR_LIGHT_BLUE, COLOR_BLACK);
    init_pair(9, COLOR_orange, COLOR_BLACK);

    noecho();
    curs_set(FALSE);
    srand(time(NULL));
    int tabagheh = 1;
    while (tabagheh <= 4) {
        clear();
        int fullMapReveal = 0; 
        

        char map[MAP_HEIGHT][MAP_WIDTH];
        char visible_map[MAP_HEIGHT][MAP_WIDTH] = {0}; 
        Room rooms[6]; 
        Player player;

        initialize_map(map);
        generate_random_map(map, rooms, 6);
         place_black_gold(map, rooms, 6);
         place_traps_in_rooms(map, rooms, 6);
          place_food_in_rooms(map, rooms, 6);
         if (tabagheh == 4) {
                place_treasure_room(map, rooms, 6);
            }
        place_secret_door(map, rooms);
        place_random_magic_items(map, rooms, 6);
        place_random_weapons(map,rooms,6);
        place_random_enemies( map,rooms,6, 5);
        
        draw_map_with_visibility(map, visible_map);

      
        refresh();

        place_player(rooms[0], &player);

        reveal_area(visible_map, player.x, player.y, 5);
        draw_map_with_visibility(map, visible_map);

        refresh();

       handle_player_movement(map, visible_map, &player, &fullMapReveal);
       

        if (map[player.y][player.x] == LAST_ROOM_MARK) {
            tabagheh++;
            mvprintw(MAP_HEIGHT + 2, 0, "Moving to floor %d...", tabagheh);
            refresh();
            napms(1000);
        } else {
            break;
        }
    }

    mvprintw(MAP_HEIGHT + 4, 0, "Game Over! Thanks for playing.");
    refresh();
    getch();
    endwin();

    return 0;
}
