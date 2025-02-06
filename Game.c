#include <ncurses.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <regex.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>

#define MAX_USERS 1000 
#define USERNAME_LENGTH 50
#define PASSWORD_LENGTH 50
#define EMAIL_LENGTH 100
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
#define MAX_SCORE_ENTRIES 1000
#define FILENAME "bbb.txt"
#define MAX_DISPLAY_ENTRIES 10
#define GOLD_COLOR 10
#define SILVER_COLOR 11
#define BRONZE_COLOR 12
#define HIGH_QUALITY_FOOD 'h' 
#define MAGIC_FOOD 'm' 
#define SPOILED_FOOD 'x'  

#define REGEN_RATE 2


typedef struct {
    char username[50];  
    int rank;           
    int total_points;  
    int gold;         
    int completed_games; 
    int experience; 
    time_t  start_time;     
    int is_current_user; 
} ScoreEntry;
ScoreEntry leaderboard[MAX_SCORE_ENTRIES];
int leaderboard_size = 0;

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


Weapon available_weapons[5] = {
    {'g', "Mace", 5, 0, 1},
    {'k', "Dagger", 12, 5, 0},
    {'a', "Magic Wand", 15, 10, 0},
    {'t', "Arrow", 5, 5, 0},
    {'w', "Sword", 10, 0, 0}
};


Weapon current_weapon = {'g', "Mace", 5, 0, 1}; 


typedef struct {
    int normal_food_count;
    int high_quality_food_count;
    int magic_food_count;
    int spoiled_food_count;
    int hunger;
} Backpack_Food;

Backpack_Food player_backpack_food = {0,0,0,0,0};


typedef struct {
    int spell_count;
    int active;       
    int remaining_moves; 
    int original_weapon_power;
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


char discovered_map[MAP_HEIGHT][MAP_WIDTH] = {0}; 
int tala=0;
int score=0;
int player_health = 100;
int player_damage_boost;
int old_game=0;

typedef struct {
    char username[USERNAME_LENGTH];
    char password[PASSWORD_LENGTH];
    char email[EMAIL_LENGTH];
} User;

User loggedInUser; 
User users[MAX_USERS];
int user_count = 0;
char* theme = "Dark"; 
int tabagheh = 1;

typedef struct {
    int x, y;              
    int health;            
    int score;             
    int gold;             
    int moves;    
    int tabagheh;         
    Weapon current_weapon; 
    Backpack_Food hunger;  
    Backpack spell;
    Weapon available_weapons[5];   
    Enemy enemies[7];    
    char username[50];    
} Player;



void preGameMenu(WINDOW *win);
void main_naghsheh(WINDOW *win);  

void load_game(const char *username, char map[MAP_HEIGHT][MAP_WIDTH], char visible_map[MAP_HEIGHT][MAP_WIDTH], Player *player) {
    char filename[100];
    sprintf(filename, "%s_save.dat", username);

    FILE *file = fopen(filename, "rb");
    if (!file) {
        mvprintw(0, 0, "No saved game found for user %s!", username);
        return;
    }

    if (fread(map, sizeof(char), MAP_HEIGHT * MAP_WIDTH, file) != MAP_HEIGHT * MAP_WIDTH) {
        mvprintw(1, 0, "Error reading map data!");
        fclose(file);
        return;
    }

    if (fread(visible_map, sizeof(char), MAP_HEIGHT * MAP_WIDTH, file) != MAP_HEIGHT * MAP_WIDTH) {
        mvprintw(2, 0, "Error reading visible map data!");
        fclose(file);
        return;
    }

    if (fread(player, sizeof(Player), 1, file) != 1) {
        mvprintw(3, 0, "Error reading player data!");
        fclose(file);
        return;
    }

    fclose(file);
    mvprintw(0, 0, "Game loaded successfully!");
}

void save_game(char map[MAP_HEIGHT][MAP_WIDTH], char visible_map[MAP_HEIGHT][MAP_WIDTH], Player *player) {
    char filename[100];
    sprintf(filename, "%s_save.dat", loggedInUser.username);

    FILE *file = fopen(filename, "wb");
    if (!file) {
        mvprintw(0, 0, "Error saving the game!");
        napms(3000);
        return;
    }

    if (fwrite(map, sizeof(char), MAP_HEIGHT * MAP_WIDTH, file) != MAP_HEIGHT * MAP_WIDTH) {
        mvprintw(1, 0, "Error writing map data!");
        fclose(file);
        napms(3000);
        return;
    }

    if (fwrite(visible_map, sizeof(char), MAP_HEIGHT * MAP_WIDTH, file) != MAP_HEIGHT * MAP_WIDTH) {
        mvprintw(2, 0, "Error writing visible map data!");
        fclose(file);
        napms(3000);
        return;
    }

    if (fwrite(player, sizeof(Player), 1, file) != 1) {
        mvprintw(3, 0, "Error writing player data!");
        fclose(file);
        napms(3000);
        return;
    }

    fclose(file);
    mvprintw(0, 0, "Game saved successfully!");
    refresh();
}


void load_leaderboard(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) return; 

    leaderboard_size = 0;
    while (fscanf(file, "%49s %d %d %d %d %d %ld",
              leaderboard[leaderboard_size].username,
              &leaderboard[leaderboard_size].rank,
              &leaderboard[leaderboard_size].total_points,
              &leaderboard[leaderboard_size].gold,
              &leaderboard[leaderboard_size].completed_games,
              &leaderboard[leaderboard_size].experience,
              &leaderboard[leaderboard_size].start_time) == 7) {
    leaderboard_size++;
    if (leaderboard_size >= MAX_SCORE_ENTRIES) break;
}

    fclose(file);
}

void save_leaderboard(const char *filename) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        printf("Error saving leaderboard!\n");
        return;
    }

    for (int i = 0; i < leaderboard_size; i++) {
        fprintf(file, "%s %d %d %d %d %d %ld\n", 
        leaderboard[i].username,
        leaderboard[i].rank,
        leaderboard[i].total_points,
        leaderboard[i].gold,
        leaderboard[i].completed_games,
        leaderboard[i].experience,
        leaderboard[i].start_time);

    }

    fclose(file);
}

void update_leaderboard() {
    load_leaderboard(FILENAME);
    int found = -1;
    time_t current_time = time(NULL);

    for (int i = 0; i < leaderboard_size; i++) {
        if (strcmp(leaderboard[i].username, loggedInUser.username) == 0) {
            found = i;
            break;
        }
    }

    if (found != -1) {
        leaderboard[found].total_points += (tala+score);
        leaderboard[found].gold += tala;
        leaderboard[found].completed_games++;
        double elapsed = difftime(current_time, leaderboard[found].start_time);
        leaderboard[found].experience = (int)(elapsed / 3600);  
    } else if (leaderboard_size < MAX_SCORE_ENTRIES) {
        strcpy(leaderboard[leaderboard_size].username,loggedInUser.username);
        leaderboard[leaderboard_size].rank = 0;
        leaderboard[leaderboard_size].total_points = (tala+score);
        leaderboard[leaderboard_size].gold = tala;
        leaderboard[leaderboard_size].completed_games = 1;
        leaderboard[leaderboard_size].start_time = current_time;
        leaderboard[leaderboard_size].experience = 0;
        leaderboard[leaderboard_size].is_current_user = 0;
        leaderboard_size++;
    }
    for (int i = 0; i < leaderboard_size - 1; i++) {
        for (int j = i + 1; j < leaderboard_size; j++) {
            if (leaderboard[j].total_points > leaderboard[i].total_points) {
             
                ScoreEntry temp = leaderboard[i];
                leaderboard[i] = leaderboard[j];
                leaderboard[j] = temp;
            }
        }
    }

 
    for (int i = 0; i < leaderboard_size; i++) {
        leaderboard[i].rank = i + 1;
    }

    save_leaderboard(FILENAME);
}

void handle_enemy_weapons_attack(Player *player, Enemy enemies[], int num_enemies,char map[MAP_HEIGHT][MAP_WIDTH],WINDOW *win) {
         bool under_attack = false;
    for (int i = 0; i < num_enemies; i++) {
        if (enemies[i].is_alive) {
            int dx = abs(enemies[i].x - player->x);
            int dy = abs(enemies[i].y - player->y);

            if (dx <= 1 && dy <= 1 ) {
               if (current_weapon.symbol=='g' || current_weapon.symbol == 'w'){
                enemies[i].health -= current_weapon.damage;
                if (enemies[i].health > 0) {
                     attron(COLOR_PAIR(3)); 
                    mvprintw(0, 0, "You attacked %c! Enemy health: %d", enemies[i].symbol, enemies[i].health);
                     attroff(COLOR_PAIR(3)); 
                } else {
                    mvprintw(0, 0, "You defeated %c! You'r score increased '5'", enemies[i].symbol);
                    enemies[i].is_alive=0;
                    score+=5;
                     map[enemies[i].y][enemies[i].x]=FLOOR_CHAR;                }
                refresh();
                

               }

                player_health -= enemies[i].damage;
                under_attack = true;
                attron(COLOR_PAIR(1));
                mvprintw(1, 0, "Attacked by %c! Health: %d", enemies[i].symbol, player_health);
                attroff(COLOR_PAIR(1)); 
                refresh();
                napms(1000);
                mvprintw(0, 0, "                                                        ");
                mvprintw(1, 0, "                                                        ");
                refresh();

                if (player_health <= 0) {
                    clear();
                   attron(COLOR_PAIR(1));   
                    mvprintw(15, 30, "Game Over! You died.");
                    attroff(COLOR_PAIR(1));  
                   int emtiyaz = (score + tala);
                    mvprintw(16, 31, "Your final score: %d", emtiyaz);
                    mvprintw(17, 30, "Press 'o' to return to the main menu.");
                    update_leaderboard();
                    refresh();
                    napms(3000);
                    int key;
                    while ((key = getch()) != 'o');
                    clear();
                    refresh();
                    tabagheh=5;
                    main_naghsheh(win);
                            
                }
            }
             if (dx <= 5 && dy <= 5 )
             {
               
               
                if (current_weapon.symbol=='t' && available_weapons[3].count>0){
                enemies[i].health -= current_weapon.damage;
                
                    available_weapons[3].count--;
                
                if (enemies[i].health > 0) {
                     attron(COLOR_PAIR(3)); 
                    mvprintw(0, 0, "You attacked %c! Enemy health: %d", enemies[i].symbol, enemies[i].health);
                     attroff(COLOR_PAIR(3)); 
                } else {
                    mvprintw(0, 0, "You defeated %c! You'r score increased '5'", enemies[i].symbol);
                    enemies[i].is_alive=0;
                    score+=5;
                     map[enemies[i].y][enemies[i].x]=FLOOR_CHAR;                }
                refresh();
                if (available_weapons[3].count<=0)
               {
                current_weapon.symbol='@';
               }

               }
               if ( available_weapons[1].count>0 && current_weapon .symbol== 'k')
               {
               enemies[i].health -= current_weapon.damage;
                    available_weapons[1].count--;
                
                if (enemies[i].health > 0) {
                     attron(COLOR_PAIR(3)); 
                    mvprintw(0, 0, "You attacked %c! Enemy health: %d", enemies[i].symbol, enemies[i].health);
                     attroff(COLOR_PAIR(3)); 
                } else {
                    mvprintw(0, 0, "You defeated %c! You'r score increased '5'", enemies[i].symbol);
                    enemies[i].is_alive=0;
                    score+=5;
                     map[enemies[i].y][enemies[i].x]=FLOOR_CHAR;                }
                refresh();
                if (available_weapons[1].count<=0)
               {
                current_weapon.symbol='@';
               }
               }
               
             }

             if (dx <= 10 && dy <= 10 ){
                 if (current_weapon.symbol=='a'
                 && available_weapons[2].count>0 
                 ){
                enemies[i].health -= current_weapon.damage;
                available_weapons[2].count--;
                enemies[i].chase_steps=0;
                if (enemies[i].health > 0) {
                     attron(COLOR_PAIR(3)); 
                    mvprintw(0, 0, "You attacked %c! Enemy health: %d", enemies[i].symbol, enemies[i].health);
                     attroff(COLOR_PAIR(3)); 
                } else {
                    mvprintw(0, 0, "You defeated %c! You'r score increased '5'", enemies[i].symbol);
                    enemies[i].is_alive=0;
                    score+=5;
                     map[enemies[i].y][enemies[i].x]=FLOOR_CHAR;                }
                refresh();
                if (available_weapons[2].count<=0)
               {
                current_weapon.symbol='@';
               }

               }
             }

        }
    }

    if (!under_attack && player_backpack_food.hunger <= 0 ) {
        player_health += REGEN_RATE;
        mvprintw(1, 0, "Player health regenerating... Current health: %d", player_health);
        refresh();
        napms(1000);
        mvprintw(1, 0, "                                                        ");
        refresh();
    }

}


void move_enemy(Enemy *enemy, Player *player, char map[MAP_HEIGHT][MAP_WIDTH]) {
    if (!enemy->is_alive) return;

    if (enemy->chase_steps == 0) return;

    if (enemy->symbol == DEMON_SYMBOL || enemy->symbol == FIRE_MONSTER_SYMBOL) return;
    
    // mvaddch( enemy->y, enemy->x, FLOOR_CHAR);

    int dx = (player->x > enemy->x) ? 1 : (player->x < enemy->x) ? -1 : 0;
    int dy = (player->y > enemy->y) ? 1 : (player->y < enemy->y) ? -1 : 0;

    int new_x = enemy->x + dx;
    int new_y = enemy->y + dy;

    if (map[new_y][new_x] == FLOOR_CHAR ||  map[new_y][new_x] == '*') {
        map[ enemy->y][enemy->x] = FLOOR_CHAR;
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

        if (map[y][x] == FLOOR_CHAR || map[y][x] == '*') {  
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

            switch (weapon_symbol) {
                case 'k': 
                    available_weapons[i + 1].count += 10;
                    mvprintw(0, 0, "You collected daggers! New total: %d", available_weapons[i + 1].count);
                    break;

                case 'a': 
                    available_weapons[i + 1].count += 8;
                    mvprintw(0, 0, "You collected magic wands! New total: %d", available_weapons[i + 1].count);
                    break;

                case 't': 
                    available_weapons[i + 1].count += 20;
                    mvprintw(0, 0, "You collected arrows! New total: %d", available_weapons[i + 1].count);
                    break;

                case 'w': 
                    available_weapons[i + 1].count = 1; 
                    mvprintw(0, 0, "You now possess a sword.");
                    break;

                default:
                    mvprintw(0, 0, "Invalid weapon symbol.");
                    break;
            }

            refresh();
            sleep(1);
            mvprintw(0, 0, "                                                       ");
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


void collect_food(char food) {
    switch (food) {
        case NORMAL_FOOD:
            if (player_backpack_food.normal_food_count < 5) {
                player_backpack_food.normal_food_count++;
               mvprintw(0 , 0,"You collected a normal food.\n");
            } else {
                 mvprintw(0 , 0,"Backpack is full for normal food!\n");
            }
            break;
        case HIGH_QUALITY_FOOD:
            if (player_backpack_food.high_quality_food_count < 5) {
                player_backpack_food.high_quality_food_count++;
               mvprintw(0 , 0,"You collected a high-quality food.\n");
            } else {
                mvprintw(0 , 0,"Backpack is full for high-quality food!\n");
            }
            break;
        case MAGIC_FOOD:
            if (player_backpack_food.magic_food_count < 5) {
                player_backpack_food.magic_food_count++;
                mvprintw(0 , 0,"You collected a magic food.\n");
            } else {
                mvprintw(0 , 0,"Backpack is full for magic food!\n");
            }
            break;
        case SPOILED_FOOD:
            if (player_backpack_food.spoiled_food_count <5) {
                player_backpack_food.spoiled_food_count++;
                mvprintw(0 , 0,"You collected a spoiled food... Be careful!\n");
            } else {
               mvprintw(0 , 0,"Backpack is full for spoiled food!\n");
            }
            break;
        default:
            break;
    }
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

    sleep(5); 
    mvprintw(MAP_HEIGHT + 2, 0, "                                                          "); 
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
                // available_weapons[4].count--;
               current_weapon.symbol = 'w';
                    strcpy(current_weapon.name, "Sword");
                    current_weapon.damage = 10;
                    current_weapon.range = 0;
                    // current_weapon.count = 0;
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
                // available_weapons[1].count--;
                current_weapon.symbol = 'k';
                    strcpy(current_weapon.name, "Dagger");
                    current_weapon.damage = 12;
                    current_weapon.range = 5;
                    // current_weapon.count = 0;
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
                // available_weapons[3].count--;
                current_weapon.symbol = 't';
                    strcpy(current_weapon.name, "Arrow");
                    current_weapon.damage = 5;
                    current_weapon.range = 5;
                    // current_weapon.count = 0;
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
                // available_weapons[2].count--;
                current_weapon.symbol = 'a';
                    strcpy(current_weapon.name, "Magic Wand");
                    current_weapon.damage = 15;
                    current_weapon.range = 10;
                    // current_weapon.count = ;
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
    mvprintw(MAP_HEIGHT + 4, 0, "Premium Food (h): %d", player_backpack_food.high_quality_food_count);
    mvprintw(MAP_HEIGHT + 5, 0, "Magic Food (m): %d", player_backpack_food.magic_food_count);
    mvprintw(MAP_HEIGHT + 6, 0, "Corrupt Food (x): %d", player_backpack_food.spoiled_food_count);

    mvprintw(MAP_HEIGHT + 7, 0, "Hunger: %d/%d", player_backpack_food.hunger, MAX_HUNGER);
    mvprintw(MAP_HEIGHT + 8, 0, "Health: %d", player_health);
    mvprintw(MAP_HEIGHT + 9, 0, "Hunger: [");
    for (int i = 0; i < player_backpack_food.hunger; i++) {
            mvprintw(MAP_HEIGHT + 9, 10 + i, "*");  
    }

        mvprintw(MAP_HEIGHT + 9, 20 +player_backpack_food.hunger, "]");  
        
    refresh();


    int ch = getch();
     switch (ch) {
         case 'n':
         if (player_backpack_food.normal_food_count > 0) {
        player_health+=5;
        player_backpack_food.normal_food_count--;
        if (player_backpack_food.hunger > 0) player_backpack_food.hunger -= 5;  
        if (player_backpack_food.hunger <= 0) player_backpack_food.hunger = 0;
         attron(COLOR_PAIR(3)); 
        mvprintw(MAP_HEIGHT + 10, 0, "Health: %d", player_health);
         attroff(COLOR_PAIR(3)); 
                refresh();
                sleep(2);} else {
                mvprintw(MAP_HEIGHT + 11, 0, "No Normal Food Left!");
            }
            break;
            case 'h': 
            if (player_backpack_food.high_quality_food_count > 0) {
                player_health += 10;
                player_backpack_food.high_quality_food_count--;
                player_backpack_food.hunger -= 7;
                if (player_backpack_food.hunger < 0) player_backpack_food.hunger = 0;
                current_weapon.damage += 2; 
                mvprintw(MAP_HEIGHT + 10, 0, "Consumed Premium Food. Attack Power Boosted!");
            } else {
                mvprintw(MAP_HEIGHT + 10, 0, "No Premium Food Left!");
            }
            break;
            case 'm':  
            if (player_backpack_food.magic_food_count > 0) {
                player_health += 15;
                player_backpack_food.magic_food_count--;
                player_backpack_food.hunger -= 10;
                if (player_backpack_food.hunger < 0) player_backpack_food.hunger = 0;
                activate_spell(&player_backpack.speed_spells); 
                mvprintw(MAP_HEIGHT + 10, 0, "Consumed Magic Food. Speed Increased!");
            } else {
                mvprintw(MAP_HEIGHT + 10, 0, "No Magic Food Left!");
            }
            break;
            case 'x':  
            if (player_backpack_food.spoiled_food_count > 0) {
                player_health -= 10; 
                if (player_health<=0)
                {
                 attron(COLOR_PAIR(1));   
                 mvprintw(15, 30, "Game Over! You died.");
                attroff(COLOR_PAIR(1));  
                update_leaderboard();
                clear();
                refresh();
                napms(3000);
                endwin();
                // preGameMenu(win);
                exit(0);
                    
                }
                player_backpack_food.spoiled_food_count--;
                mvprintw(MAP_HEIGHT + 10, 0, "Consumed Corrupt Food. Health Decreased!");
            } else {
                mvprintw(MAP_HEIGHT + 10, 0, "No Corrupt Food Left!");
            }
            break;

        default:
            mvprintw(MAP_HEIGHT + 10, 0, "Invalid Choice.");
            break;
    }
      refresh();
    sleep(2);

    
    for (int i = 2; i <= 10; i++) {
        move(MAP_HEIGHT + i, 0);
        clrtoeol();
    }
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

                player_backpack.damage_spells.active = 1;
                player_backpack.damage_spells.remaining_moves = 10;

               player_backpack.damage_spells.original_weapon_power=current_weapon.damage; 
                current_weapon.damage *= 2;


                mvprintw(MAP_HEIGHT + 6, 0, "Damage spell activated! Weapon power doubled.");
                refresh();
                sleep(2);
                mvprintw(MAP_HEIGHT + 6, 0, "                                                       "); 
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
        int room_index = 1 + rand() % (room_count - 1);  
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

    int colon_x;
    do {
        colon_x = treasure_room.x + rand() % treasure_room.width;
    } while (map[treasure_room.y + treasure_room.height - 1][colon_x] != '_');
    map[treasure_room.y + treasure_room.height - 1][colon_x] = 'o';  
}

void place_enchant_room(char map[MAP_HEIGHT][MAP_WIDTH], Room rooms[], int room_count) {
  
    Room enchant_room = rooms[0];

    for (int y = enchant_room.y; y < enchant_room.y + enchant_room.height; y++) {
        for (int x = enchant_room.x; x < enchant_room.x + enchant_room.width; x++) {
            if (map[y][x] == FLOOR_CHAR) {
                map[y][x] = 'e'; 
            }
        }
    }
    char magic_symbols[3] = {SPELL_HEALTH_CHAR,SPELL_SPEED_CHAR,SPELL_DAMAGE_CHAR };
     int spell_count = 5 + rand() % 3;  
    for (int i = 0; i < spell_count; i++) {
        int tx, ty;
        do {
            tx = enchant_room.x + 1 + rand() % (enchant_room.width - 2);
            ty = enchant_room.y + 1 + rand() % (enchant_room.height - 2);
        } while (map[ty][tx] != 'e'); 

        char selected_symbol = magic_symbols[rand() % 3];  
        map[ty][tx] = selected_symbol;
    }
    
}

void place_food_in_rooms(char map[MAP_HEIGHT][MAP_WIDTH], Room rooms[], int num_rooms) {
    int item_count = 5; 

    char food_types[] = {NORMAL_FOOD, HIGH_QUALITY_FOOD, MAGIC_FOOD, SPOILED_FOOD};

    for (int i = 0; i < item_count; i++) {
        int placed = 0;

        while (!placed) {
            int room_index = rand() % num_rooms;
            Room selected_room = rooms[room_index];

            int x = selected_room.x + 1 + rand() % (selected_room.width - 2);
            int y = selected_room.y + 1 + rand() % (selected_room.height - 2);

            if (map[y][x] == FLOOR_CHAR) {
                char food = food_types[rand() % 4]; 
                map[y][x] = food;
                placed = 1;
            }
        }
    }
}


void place_traps_in_rooms(char map[MAP_HEIGHT][MAP_WIDTH], Room rooms[], int num_rooms) {
    int selected_rooms[3] = {-1, -1, -1}; 
    int trap_rooms_count = 0;


    while (trap_rooms_count < 3 && trap_rooms_count < num_rooms) {
        int room_index = rand() % num_rooms;

        int already_selected = 0;
        for (int i = 0; i < trap_rooms_count; i++) {
            if (selected_rooms[i] == room_index) {
                already_selected = 1;
                break;
            }
        }
        if (already_selected) continue;

        selected_rooms[trap_rooms_count++] = room_index;
        Room selected_room = rooms[room_index];

        int trap_count = rand() % 2; 
        for (int i = 0; i < trap_count; i++) {
            int tx, ty;
            do {
                tx = selected_room.x + 1 + rand() % (selected_room.width - 2);
                ty = selected_room.y + 1 + rand() % (selected_room.height - 2);
            } while (map[ty][tx] != FLOOR_CHAR);
            map[ty][tx] = TRAP_HIDDEN_CHAR; 
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
                     if (theme == "Light") {
                        attron(COLOR_PAIR(4));
                    } else {
                        attron(COLOR_PAIR(1)); 
                    }
                } else if (map[i][j] == CORRIDOR_CHAR || map[i][j] == BLACK_GOLD_CHAR) {
                    attron(COLOR_PAIR(2));
                } else if (map[i][j] == FLOOR_CHAR || map[i][j] == TRAP_HIDDEN_CHAR || map[i][j] == 'e') {
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

void move_player_with_visibility(char map[MAP_HEIGHT][MAP_WIDTH],char visible_map[MAP_HEIGHT][MAP_WIDTH], Player *player, int dx, int dy,WINDOW *win) {
    int step_size = ( player_backpack.speed_spells.active > 0) ? 2 : 1; 
    int new_x = player->x + dx * step_size;
    int new_y = player->y + dy * step_size;

    player->moves+=step_size; 
    
    if (player_backpack.damage_spells.active) {
    player_backpack.damage_spells.remaining_moves--;

    if (player_backpack.damage_spells.remaining_moves <= 0) {
        player_backpack.damage_spells.active = 0;
        current_weapon.damage = player_backpack.damage_spells.original_weapon_power;

        mvprintw(0, 0, "Damage spell expired! Weapon power restored.");
        refresh();
        napms(1000);
        mvprintw(0, 0, "                                                      ");
        refresh();
    }
}


    if (player->moves % 20 == 0) { 
        player_backpack_food.hunger++;
        if (player_backpack_food.hunger > MAX_HUNGER) 
        {player_backpack_food.hunger = MAX_HUNGER;
         player_health--;
        }


        mvprintw(0, 0, "Hunger increased! Current Hunger: %d", player_backpack_food.hunger);
        if (player_backpack_food.hunger == MAX_HUNGER){
             attron(COLOR_PAIR(1)); 
             mvprintw(1, 0, "Health dicreased! Currnt Health: %d", player_health);
              attroff(COLOR_PAIR(1)); 
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
            target == NORMAL_FOOD ||target== DAGGER_CHAR || target == MAGIC_WAND_CHAR ||target== ARROW_CHAR || 
            target== SWORD_CHAR || target == SPOILED_FOOD || target == MAGIC_FOOD || target == HIGH_QUALITY_FOOD ||
             target == TRAP_DISCOVERED_CHAR || target == 'o'|| target == 'e') {
            
            if (target== DAGGER_CHAR || target == MAGIC_WAND_CHAR ||target== ARROW_CHAR || target== SWORD_CHAR )
            {
                map[new_y][new_x] = FLOOR_CHAR;
                collect_weapon(target);
            }
            
            if (target == NORMAL_FOOD || target == SPOILED_FOOD || target == MAGIC_FOOD || target == HIGH_QUALITY_FOOD)
            {
                map[new_y][new_x] = FLOOR_CHAR;
                collect_food(target);
            }
            if (target == 'e')
            {
                player_health-=1;
                 attron(COLOR_PAIR(1)); 
                mvprintw(0, 0, "You are in enchant room! Your  Health %d",player_health);
                 attroff(COLOR_PAIR(1)); 
                refresh();
                napms(1000);
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
                     attron(COLOR_PAIR(1)); 
                    mvprintw(0, 0, "Trap discovered! Health -10. You'r Health is %d", player_health); 
                     attroff(COLOR_PAIR(1)); 
                    refresh();
                    sleep(1); 
                    mvprintw(0, 0, "                                                     "); 
                    refresh();
                }
                if (player_health <= 0) {
                    clear();
                    refresh();
                    update_leaderboard();
                   attron(COLOR_PAIR(1));   
                    mvprintw(15, 30, "Game Over! You died.");
                    attroff(COLOR_PAIR(1)); 
                     int emtiyaz = (score + tala);
                    mvprintw(16, 31, "Your final score: %d", emtiyaz);
                    mvprintw(17, 30, "Press 'o' to return to the main menu.");
                    update_leaderboard();
                    refresh();
                    napms(3000);
                    int key;
                    while ((key = getch()) != 'o');
                    clear();
                    refresh();
                    tabagheh=5;
                    main_naghsheh(win);
                    
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


void handle_player_movement(char map[MAP_HEIGHT][MAP_WIDTH],char visible_map[MAP_HEIGHT][MAP_WIDTH], Player *player, int *fullMapReveal,WINDOW *win) {
    int gameRunning = 1;
    int key;
    keypad(stdscr, TRUE);

    while (gameRunning) {
        key = getch();

        if (key == 's' || key == 'S')
        {   
            player->tabagheh = tabagheh;
            player->health = player_health;
            player->gold= tala;
            player->score = (score+tala);
            player->hunger = player_backpack_food;
            player->current_weapon = current_weapon;
            player->spell = player_backpack;
           for (int i = 0; i < 5; i++) {
        player->available_weapons[i] = available_weapons[i];}
           for (int i = 0; i < 7; i++)
           {
            player->enemies[i]= enemies[i];
           }
           
            strcpy(player->username, loggedInUser.username);
             save_game(map, visible_map,player);
             clear();
             mvprintw(18,30 , "Game saved successfully!");
             mvprintw(19, 30, "Press 'o' to return to the main menu.");
             napms(3000);
             gameRunning = 0;
             int key;
             while ((key = getch()) != 'o');
             clear();
             refresh();
             tabagheh=5;
            main_naghsheh(win);
        }
        

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
        

        if (map[player->y][player->x] == LAST_ROOM_MARK || map[player->y][player->x] == SECRET_DOOR_DISCOVERED ) {
            break;
        }
        if (map[player->y][player->x] == 'o') {
            clear();
            attron(COLOR_PAIR(3));   
             mvprintw(15, 30, "You found the treasure room! Game Over.");
            attroff(COLOR_PAIR(3));  
            int emtiyaz = (score + tala+50);
            mvprintw(16, 31, "Your final score: %d", emtiyaz);
            mvprintw(17, 30, "Press 'o' to return to the main menu.");
            update_leaderboard();
            refresh();
            napms(3000);
             gameRunning = 0;
             int key;
             while ((key = getch()) != 'o');
             clear();
            refresh();
             tabagheh=5;
            main_naghsheh(win);
        }


        switch (key) {
            case '7': move_player_with_visibility(map, visible_map, player, -1, -1,win); break;
            case '8': move_player_with_visibility(map, visible_map, player, 0, -1,win); break;
            case '9': move_player_with_visibility(map, visible_map, player, 1, -1,win); break;
            case '4': move_player_with_visibility(map, visible_map, player, -1, 0,win); break;
            case '6': move_player_with_visibility(map, visible_map, player, 1, 0,win); break;
            case '1': move_player_with_visibility(map, visible_map, player, -1, 1,win); break;
            case '2': move_player_with_visibility(map, visible_map, player, 0, 1,win); break;
            case '3': move_player_with_visibility(map, visible_map, player, 1, 1,win); break;
        }
        
    

        refresh();
        for (int i = 0; i < 5; i++) {
            if (enemies[i].is_alive && is_player_in_room(player, enemies[i].room)) {
                move_enemy(&enemies[i], player, map);
                mvaddch(enemies[i].y, enemies[i].x, enemies[i].symbol);
            }
            }

        handle_enemy_weapons_attack(player, enemies, 5, map,win);
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


    int wall_object_count = rand() % 3;
    for (int i = 0; i < wall_object_count; i++) {
        int wx, wy;
        do {
            if (rand() % 2) { 
                wy = (rand() % 2) ? room.y : room.y + room.height - 1;
                wx = room.x + 1 + rand() % (room.width - 2);
            } else { 
                wx = (rand() % 2) ? room.x : room.x + room.width - 1;
                wy = room.y + 1 + rand() % (room.height - 2);
            }
        } while (map[wy][wx] != WALL_HORIZONTAL && map[wy][wx] != WALL_VERTICAL);
        map[wy][wx] = WALL_OBJECT_CHAR;
    }
}

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


void main_naghsheh(WINDOW *win) {
    clear();
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
    
    while (tabagheh <= 4) {
        clear();
        int fullMapReveal = 0; 
        

        char map[MAP_HEIGHT][MAP_WIDTH];
        char visible_map[MAP_HEIGHT][MAP_WIDTH] = {0}; 
        Room rooms[6]; 
        Player player;

        initialize_map(map);
        if (old_game)
        {
            load_game(loggedInUser.username,map,visible_map,&player);
            draw_map_with_visibility(map, visible_map);
            mvaddch(player.y, player.x, current_weapon.symbol);
            refresh();
            tabagheh=player.tabagheh  ;
            player_health =  player.health ;
           tala =  player.gold ;
            score=(player.score - player.gold)  ;
             player_backpack_food =player.hunger ;
             current_weapon = player.current_weapon;
            player_backpack =  player.spell;
           for (int i = 0; i < 5; i++) {
         available_weapons[i] = player.available_weapons[i];}
         for (int i = 0; i < 7; i++)
           {
           enemies[i] =  player.enemies[i] ;
           }
        }
        else{
        
        generate_random_map(map, rooms, 6);
         place_black_gold(map, rooms, 6);
         place_traps_in_rooms(map, rooms, 6);
          place_food_in_rooms(map, rooms, 6);
         if (tabagheh == 4) {
                place_treasure_room(map, rooms, 6);
            }
        if (tabagheh>=2)
        {
           place_enchant_room(map,rooms,6);
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
        }

       handle_player_movement(map, visible_map, &player, &fullMapReveal,win);
       

        if (map[player.y][player.x] == LAST_ROOM_MARK || map[player.y][player.x] == SECRET_DOOR_DISCOVERED) {
            tabagheh++;
            mvprintw(MAP_HEIGHT + 2, 0, "Moving to floor %d...", tabagheh);
            refresh();
            napms(1000);
        } else {
            break;
        }
    }
   
    update_leaderboard();
    preGameMenu(win);
    endwin();
    exit(0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void display_leaderboard(WINDOW *win) {
    int start_row = 1;
    int key;
    int page_number = 0;
    const int entries_per_page = 10;
   int total_pages = ((leaderboard_size/10)+2);
    load_leaderboard(FILENAME);
    initscr();
    start_color();

    if (can_change_color()) {
        init_color(GOLD_COLOR, 1000, 843, 0);   
        init_color(SILVER_COLOR, 752, 752, 752);     
        init_color(BRONZE_COLOR, 804, 498, 196);  
    }

    init_pair(10, GOLD_COLOR, COLOR_BLACK);  
    init_pair(11, SILVER_COLOR, COLOR_BLACK);    
    init_pair(12, BRONZE_COLOR, COLOR_BLACK); 

    do {
        werase(win);
        box(win, 0, 0);
        
        wattron(win, A_BOLD);
        mvwprintw(win, start_row, 2, "=== Leaderboard === (Page %d of %d)", page_number + 1, total_pages);
        wattroff(win, A_BOLD);

        mvwprintw(win, start_row + 1, 2, "          Rank  Username       Points  Gold  Games  Experience  Start_time");
        

        int display_count = 0;
        int row = 0;
        int start_index = page_number * entries_per_page;
        int end_index = start_index + entries_per_page;

        if (end_index > leaderboard_size) {
            end_index = leaderboard_size;
        }

        for (int i = start_index; i < end_index; i++) {
            row = start_row + 3 + display_count;

            if (leaderboard[i].rank == 1) wattron(win, COLOR_PAIR(10));
            else if (leaderboard[i].rank == 2) wattron(win, COLOR_PAIR(11));
            else if (leaderboard[i].rank== 3) wattron(win, COLOR_PAIR(12));
            if (leaderboard[i].rank == 1) mvwprintw(win, start_row + 3, 2, "Legend->");
            else if (leaderboard[i].rank == 2)mvwprintw(win, start_row + 4, 2, "Champion->");
            else if (leaderboard[i].rank== 3)mvwprintw(win, start_row + 5, 2, "GOAT->");

            if (strcmp(leaderboard[i].username, loggedInUser.username) == 0) {
                wattron(win, A_BOLD | A_REVERSE);
            }

            mvwprintw(win, row, 12, "%d |%s         |%d      |%d    |%d     |%d          |%ld",
                      leaderboard[i].rank,
                      leaderboard[i].username,
                      leaderboard[i].total_points,
                      leaderboard[i].gold,
                      leaderboard[i].completed_games,
                      leaderboard[i].experience,
                      leaderboard[i].start_time);

            wattroff(win, A_BOLD | A_REVERSE);
            wattroff(win, COLOR_PAIR(10) | COLOR_PAIR(11) | COLOR_PAIR(12));

            display_count++;
        }

        mvwprintw(win, start_row + entries_per_page + 5, 2, "Use 4 (LEFT) / 6 (RIGHT) keys to scroll, 'q' to return.");
        wrefresh(win);

        key = wgetch(win);

        if (key == '6' && page_number < total_pages - 1) {
            page_number++;
        } else if (key == '4' && page_number > 0) {
            page_number--;
        }

    } while (key != 'q');
}

const char* retrieveEmail(const char *username) {
    for (int i = 0; i < user_count; i++) {
        if (strcmp(users[i].username, username) == 0) {
            return users[i].email;
        }
    }
    return NULL; 
}

int isValidPassword(char* password) {
    if (strlen(password) < 7) return 0;
    int hasDigit = 0, hasUpper = 0, hasLower = 0;
    for (int i = 0; i < strlen(password); i++) {
        if (isdigit(password[i])) hasDigit = 1;
        if (isupper(password[i])) hasUpper = 1;
        if (islower(password[i])) hasLower = 1;
    }
    return hasDigit && hasUpper && hasLower;
}

void profileMenu(WINDOW *win) {
    char *menu[] = {
        "1. View Profile Information",
        "2. Change Password",
        "3. Back to Pre-Game Menu"
    };
    int n_options = sizeof(menu) / sizeof(menu[0]);
    int highlight = 0;
    int choice = -1;
    int key;

    while (1) {
        werase(win);
        box(win, 0, 0);

        mvwprintw(win, 1, 2, "=== Profile Menu ===");

        
        for (int i = 0; i < n_options; i++) {
            if (i == highlight) {
                wattron(win, A_REVERSE);
                mvwprintw(win, i + 3, 2, "%s", menu[i]);
                wattroff(win, A_REVERSE);
            } else {
                mvwprintw(win, i + 3, 2, "%s", menu[i]);
            }
        }

        wrefresh(win);
        key = wgetch(win);

        switch (key) {
            case KEY_UP:
                highlight = (highlight - 1 + n_options) % n_options;
                break;
            case KEY_DOWN:
                highlight = (highlight + 1) % n_options;
                break;
            case 10: 
                choice = highlight;
                break;
        }

        if (choice != -1) {
            switch (choice) {
                case 0: 
                    werase(win);
                    box(win, 0, 0);
                    mvwprintw(win, 1, 2, "=== Profile Information ===");
                    mvwprintw(win, 3, 2, "Username: %s", loggedInUser.username);
                    mvwprintw(win, 4, 2, "Email: %s", loggedInUser.email);
                    mvwprintw(win, 6, 2, "Press any key to return...");
                    wrefresh(win);
                    wgetch(win);
                    break;

                case 1: 
                    werase(win);
                    box(win, 0, 0);
                    mvwprintw(win, 1, 2, "=== Change Password ===");
                    char newPassword[PASSWORD_LENGTH];
                    mvwprintw(win, 3, 2, "Enter new password: ");
                    wrefresh(win);
                    echo();
                    wgetnstr(win, newPassword, PASSWORD_LENGTH);

                
                    if (!isValidPassword(newPassword)) {
                        mvwprintw(win, 5, 2, "Error: Password must be at least 7 characters.");
                    } else {
                        strncpy(loggedInUser.password, newPassword, PASSWORD_LENGTH);
                        mvwprintw(win, 5, 2, "Password updated successfully!");
                    }
                    noecho();
                    wrefresh(win);
                    wgetch(win);
                    break;

                case 2:
                    return;
            }
            choice = -1; 
        }
    }
}

void settingsMenu(WINDOW *win) {
    char *menu[] = {
        "1. Change Difficulty",
        "2. Change Color Theme",
        "3. Select Music",
        "4. Back to Pre-Game Menu"
    };
    int n_options = sizeof(menu) / sizeof(menu[0]);
    int highlight = 0;
    int choice = -1;
    int key;

   
    char *difficulty = "Medium";
    char *selectedMusic = "Music 1";

    char *musicList[] = { "Music 1", "Music 2", "Music 3", "Music 4" };
    int n_music = sizeof(musicList) / sizeof(musicList[0]);

    
    while (1) {
        werase(win);
        box(win, 0, 0);

        mvwprintw(win, 1, 2, "=== Settings Menu ===");

        for (int i = 0; i < n_options; i++) {
            if (i == highlight) {
                wattron(win, A_REVERSE);
                mvwprintw(win, i + 3, 2, "%s", menu[i]);
                wattroff(win, A_REVERSE);
            } else {
                mvwprintw(win, i + 3, 2, "%s", menu[i]);
            }
        }

    
        mvwprintw(win, n_options + 5, 2, "Current Settings:");
        mvwprintw(win, n_options + 6, 4, "Difficulty: %s", difficulty);
        mvwprintw(win, n_options + 7, 4, "Theme: %s", theme);
        mvwprintw(win, n_options + 8, 4, "Music: %s", selectedMusic);

        wrefresh(win);
        key = wgetch(win);

        switch (key) {
            case KEY_UP:
                highlight = (highlight - 1 + n_options) % n_options;
                break;
            case KEY_DOWN:
                highlight = (highlight + 1) % n_options;
                break;
            case 10: 
                choice = highlight;
                break;
        }

        if (choice != -1) {
            switch (choice) {
                case 0: 
                    werase(win);
                    mvwprintw(win, 1, 2, "Select Difficulty:");
                    mvwprintw(win, 3, 2, "1. Easy");
                    mvwprintw(win, 4, 2, "2. Medium");
                    mvwprintw(win, 5, 2, "3. Hard");
                    wrefresh(win);
                    key = wgetch(win);
                    if (key == '1') 
                    {difficulty = "Easy";
                    player_health=300;}
                    else if (key == '2'){difficulty = "Medium";
                    player_health=200;} 
                    else if (key == '3'){difficulty = "Hard";
                    player_health=100;} 
                    break;

                case 1: 
                    werase(win);
                    mvwprintw(win, 1, 2, "Select Color Theme:");
                    mvwprintw(win, 3, 2, "1. Light");
                    mvwprintw(win, 4, 2, "2. Dark");
                    wrefresh(win);
                    key = wgetch(win);
                    if (key == '1')theme = "Light";
                    else if (key == '2') theme = "Dark";
                    break;

                case 2: 
                    werase(win);
                    mvwprintw(win, 1, 2, "Select Music:");
                    for (int i = 0; i < n_music; i++) {
                        mvwprintw(win, i + 3, 2, "%d. %s", i + 1, musicList[i]);
                    }
                    wrefresh(win);
                    key = wgetch(win);
                    int musicIndex = key - '1'; 
                    if (musicIndex >= 0 && musicIndex < n_music) {
                        selectedMusic = musicList[musicIndex];
                    }
                    break;

                case 3:
                    return;
            }
            choice = -1; 
        }
    }
}

void preGameMenu(WINDOW *win) {
    char *menu[] = {
        "1. New Game",
        "2. Continue Game",
        "3. Leaderboard",
        "4. Settings",
        "5. Profile",
        "6. Exit"
    };
    int n_options = sizeof(menu) / sizeof(menu[0]);
    int highlight = 0;
    int choice = -1;
    int key;

    start_color();
    init_pair(1, COLOR_CYAN, COLOR_BLACK); 
    init_pair(2, COLOR_BLACK, COLOR_WHITE); 

    while (1) {
        werase(win);
        box(win, 0, 0);

        wattron(win, COLOR_PAIR(1));
        mvwprintw(win, 1, 2, "=== Pre-Game Menu ===");
        wattroff(win, COLOR_PAIR(1));

        for (int i = 0; i < n_options; i++) {
            if (i == highlight) {
                wattron(win, COLOR_PAIR(2));
                mvwprintw(win, i + 3, 2, "%s", menu[i]);
                wattroff(win, COLOR_PAIR(2));
            } else {
                mvwprintw(win, i + 3, 2, "%s", menu[i]);
            }
        }

        wrefresh(win);
        key = wgetch(win);

        switch (key) {
            case KEY_UP:
                highlight = (highlight - 1 + n_options) % n_options;
                break;
            case KEY_DOWN:
                highlight = (highlight + 1) % n_options;
                break;
            case 10: 
                choice = highlight;
                break;
        }

        if (choice != -1) {
            break;
        }
    }

    werase(win);
    box(win, 0, 0);
    wattron(win, COLOR_PAIR(1));
    mvwprintw(win, 1, 2, "You selected: %s", menu[choice]);
    wattroff(win, COLOR_PAIR(1));
    wrefresh(win);
    napms(2000);

    switch (choice) {
        case 0:
            mvwprintw(win, 3, 2, "Starting a New Game...");
            main_naghsheh(win);
            break;
        case 1:
            mvwprintw(win, 3, 2, "Continuing the Game...");
             old_game = 1;
                 main_naghsheh(win);
            break;
        case 2:
            mvwprintw(win, 3, 2, "Displaying Leaderboard...");
            display_leaderboard(win);
            break;
        case 3:
            mvwprintw(win, 3, 2, "Opening Settings...");
            settingsMenu(win);
            break;
        case 4:
            mvwprintw(win, 3, 2, "Opening Profile...");
            profileMenu(win);
            break;
        case 5:
            mvwprintw(win, 3, 2, "Exiting to Main Menu...");
            wrefresh(win);
            napms(2000);
            return;
    }
    wrefresh(win);
    napms(2000);
    preGameMenu(win); 
}

int usernameExists(char* username) {
  for (int i = 0; i < user_count; i++) {
        if (strcmp(users[i].username, username) == 0) {
            return 1;
        }
    }
    return 0;
    
}

int authenticateUser(char* username, char* password) {
    for (int i = 0; i < user_count; i++) {
        if (strcmp(users[i].username, username) == 0 && strcmp(users[i].password, password) == 0) {
            return 1;
        } 
    }
    return 0;
}

char* retrievePassword(char* email) {
    for (int i = 0; i < user_count; i++) {
        if (strcmp(users[i].email, email) == 0) {
            return users[i].password;
        }
    }
    
    return NULL;
}

void userLogin(WINDOW *win) {
    char username[USERNAME_LENGTH];
    char password[PASSWORD_LENGTH];
    char email[EMAIL_LENGTH];
    int choice;
    char input[10];
     int n_options = 3; 

    while (1) {
        werase(win);
        mvwprintw(win, 0, 0, "User Login");
        mvwprintw(win, 1, 0, "-----------");

        mvwprintw(win, 3, 0, "1. Login with username and password");
        mvwprintw(win, 4, 0, "2. Login as Guest");
        mvwprintw(win, 5, 0, "3. Forgot Password");
        mvwprintw(win, 6, 0, "Enter your choice: ");
        wrefresh(win);

        flushinp(); 

        
       wgetnstr(win, input, sizeof(input)); 
       if (isdigit(input[0])) {             
       choice = atoi(input);           
       } else {
       choice = -1;                  
       }

       if (choice <= 0 || choice > n_options) { 
        mvwprintw(win, 8, 0, "Invalid choice. Try again.");
        wrefresh(win);
        napms(2000);
       } else {
    
       }

        if (choice == 1) {
            mvwprintw(win, 8, 0, "Enter username (Press 'q' or ESC to exit): ");
            wrefresh(win);
            echo();
            wscanw(win, "%s", username);
            if (strcmp(username, "q") == 0 || username[0] == 27) {
            return; 
        }

            mvwprintw(win, 9, 0, "Enter password (Press 'q' or ESC to exit): ");
            wrefresh(win);
            noecho();
            wscanw(win, "%s", password);
            if (strcmp(password, "q") == 0 || password[0] == 27) {
            return; 
        }

            if (authenticateUser(username, password)) {
                getch();
                mvwprintw(win, 11, 0, "Login successful! Press any key to continue to the game menu...");
                wrefresh(win);

                strncpy(loggedInUser.username, username, USERNAME_LENGTH);
                strncpy(loggedInUser.password, password, PASSWORD_LENGTH);
                strncpy(loggedInUser.email, retrieveEmail(username), EMAIL_LENGTH);
                
                getch();
                preGameMenu(win); 
            } else {
                mvwprintw(win, 11, 0, "Invalid username or password. Try again.");
                wrefresh(win);
                napms(2000);
            }
        } else if (choice == 2) {
            mvwprintw(win, 8, 0, "Logged in as Guest. Press any key to continue...");
            wrefresh(win);
            strncpy(loggedInUser.username, "Guest", USERNAME_LENGTH);
            loggedInUser.email[0] = '\0'; 
            getch();
            preGameMenu(win);
        }
         else if (choice == 3) { 
            mvwprintw(win, 8, 0, "Enter your email (Press 'q' or ESC to exit): ");
            wrefresh(win);
            echo();
            wscanw(win, "%s", email);

            if (strcmp(email, "q") == 0 || email[0] == 27) {
                return;
    }

    char* retrievedPassword = retrievePassword(email);
    if (retrievedPassword) {
       
        werase(win);
        mvwprintw(win, 0, 0, "Password Recovery");
        mvwprintw(win, 1, 0, "------------------");
        mvwprintw(win, 3, 0, "Your password is: %s", retrievedPassword);
        mvwprintw(win, 5, 0, "Press any key to return to the main menu...");
        wrefresh(win);
        getch(); 
        //show_menu(win, 1, choices, n_choices);
    } else {
        
        mvwprintw(win, 10, 0, "Email not found. Try again.");
        wrefresh(win);
        napms(2000);
    }
}

        else {
            mvwprintw(win, 8, 0, "Invalid choice. Try again.");
            wrefresh(win);
            napms(2000);
        }
    }
}

int isValidEmail(const char *email) {
    regex_t regex;
    int reti; 
    const char *pattern = "^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$"; 
    reti = regcomp(&regex, pattern, REG_EXTENDED);
    if (reti) { return 0; }
    reti = regexec(&regex, email, 0, NULL, 0);
    regfree(&regex); 
    return !reti;
}

void loadUsers() {
    FILE* file = fopen("userss.dat", "r");
    if (file == NULL) {
        return;  
    }

    while (fscanf(file, "%s %s %s\n", users[user_count].username, users[user_count].password, users[user_count].email) == 3) {
        user_count++;
        if (user_count >= MAX_USERS) break;
    }

    fclose(file);
}

void saveUser(User user) {
    FILE* file = fopen("userss.dat", "a");
    if (file == NULL) {
        return;
    }
    fprintf(file, "%s %s %s\n", user.username, user.password, user.email);
    fclose(file);
    users[user_count++] = user; 
}

void generateRandomPassword(char* password, int length) {
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    for (int i = 0; i < length - 1; i++) {
        password[i] = charset[rand() % (sizeof(charset) - 1)];
    }
    password[length - 1] = '\0';
}

void createUser(WINDOW *win) {
    User user;
    char input[EMAIL_LENGTH];

    while (1) {
        werase(win);
        mvwprintw(win, 0, 0, "User Creation Form");
        mvwprintw(win, 1, 0, "-------------------");

        mvwprintw(win, 3, 0, "Enter username (Press 'q' or ESC to exit): ");
        wrefresh(win);
        echo();
        wscanw(win, "%s", input);

       
        if (strcmp(input, "q") == 0 || input[0] == 27) {
            return; 
        }

        strncpy(user.username, input, USERNAME_LENGTH);

        if (usernameExists(user.username)) {
            mvwprintw(win, 4, 0, "Username already exists. Try again.");
            wrefresh(win);
            napms(2000);
            continue;
        }

        break;
    }

    while (1) {
        mvwprintw(win, 5, 0, "Do you want to generate a random password? (y/n): ");
        wrefresh(win);
        char choice = wgetch(win);

        if (choice == 'y' || choice == 'Y') {
            generateRandomPassword(user.password, PASSWORD_LENGTH);
            mvwprintw(win, 6, 0, "Generated password: %s", user.password);
            wrefresh(win);
            napms(2000);
            break;
        } else if (choice == 'n' || choice == 'N') {
            while (1) {
                mvwprintw(win, 6, 0, "Enter password (Press 'q' or ESC to exit): ");
                wrefresh(win);
                wscanw(win, "%s", input);

              
                if (strcmp(input, "q") == 0 || input[0] == 27) {
                    return; 
                }

                strncpy(user.password, input, PASSWORD_LENGTH);

                if (!isValidPassword(user.password)) {
                    werase(win);
                    mvwprintw(win, 0, 0, "Error: Password must be at least 7 characters long and contain one digit, one uppercase letter, and one lowercase letter.");
                    wrefresh(win);
                    napms(2000);
                    continue;
                }

                break;
            }
            break;
        }
    }

    while (1) {
        mvwprintw(win, 8, 0, "Enter email (Press 'q' or ESC to exit): ");
        wrefresh(win);
        wscanw(win, "%s", input);

        if (strcmp(input, "q") == 0 || input[0] == 27) {
            return;
        }

        strncpy(user.email, input, EMAIL_LENGTH);

        if (!isValidEmail(user.email)) {
            werase(win);
            mvwprintw(win, 0, 0, "Error: Invalid email format. Try again.");
            wrefresh(win);
            napms(2000);
            continue;
        }

        break;
    }

    saveUser(user);
    mvwprintw(win, 10, 0, "User created successfully. Returning to main menu...");
    wrefresh(win);
    napms(2000);
    return; 
}

void dynamicMenu(WINDOW *win, char *menu[], int n_options) {
    int highlight = 0;
    int choice = -1;
    int key;

    start_color();
    init_pair(1, COLOR_CYAN, COLOR_BLACK); 
    init_pair(2, COLOR_BLACK, COLOR_WHITE);

    while (1) {
        werase(win);
        box(win, 0, 0);

        wattron(win, COLOR_PAIR(1));
        mvwprintw(win, 1, 2, "=== Select an option ===");
        wattroff(win, COLOR_PAIR(1));

        for (int i = 0; i < n_options; i++) {
            if (i == highlight) {
                wattron(win, COLOR_PAIR(2));
                mvwprintw(win, i + 3, 2, "%s", menu[i]);
                wattroff(win, COLOR_PAIR(2));
            } else {
                mvwprintw(win, i + 3, 2, "%s", menu[i]);
            }
        }

        wrefresh(win);
        key = wgetch(win);

        switch (key) {
            case KEY_UP:
                highlight = (highlight - 1 + n_options) % n_options;
                break;
            case KEY_DOWN:
                highlight = (highlight + 1) % n_options;
                break;
            case 10: 
                choice = highlight;
                break;
        }

        if (choice != -1) {
            break;
        }
    }

    werase(win);
    box(win, 0, 0);
    wattron(win, COLOR_PAIR(1));
    mvwprintw(win, 1, 2, "You selected: %s", menu[choice]);
    wattroff(win, COLOR_PAIR(1));
    wrefresh(win);
    napms(2000);

    if (choice == 0) {
        createUser(win);
    } else if (choice == 1) {
        userLogin(win);
    } else if (choice == n_options - 1) {
        endwin();
        exit(0);
    }
}

int main() {
    loadUsers(); 
    char *menu[] = {
        "1. Create User",
        "2. User Login",
        "3. Exit"
    };

    int n_options = sizeof(menu) / sizeof(menu[0]);
    
    initscr();
    start_color();
    init_pair(1, COLOR_GREEN, COLOR_BLACK); 
    init_pair(2, COLOR_YELLOW, COLOR_BLACK); 
    init_pair(3, COLOR_BLACK, COLOR_YELLOW); 
    cbreak();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);

    int height = 25, width = 100, start_y = 5, start_x = 15;
    WINDOW *menu_win = newwin(height, width, start_y, start_x);
    keypad(menu_win, TRUE);

    while (1) {
        dynamicMenu(menu_win, menu, n_options);
    }

    delwin(menu_win);
    endwin();
    return 0;
}
