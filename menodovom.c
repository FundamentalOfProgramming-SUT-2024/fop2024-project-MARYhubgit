#include <ncurses.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <regex.h>
#include <time.h>

#define MAX_USERS 1000 
#define USERNAME_LENGTH 50
#define PASSWORD_LENGTH 50
#define EMAIL_LENGTH 100

typedef struct {
    char username[USERNAME_LENGTH];
    char password[PASSWORD_LENGTH];
    char email[EMAIL_LENGTH];
} User;

User loggedInUser; 
User users[MAX_USERS];
int user_count = 0;

typedef struct {
    char username[30];
    int score;
    int gold;
    int gamesFinished;
    time_t firstGameTime;
} PlayerStats;

PlayerStats leaderboard[MAX_USERS];
int player_count = 0;


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
    char *theme = "Light";
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
                    if (key == '1') difficulty = "Easy";
                    else if (key == '2') difficulty = "Medium";
                    else if (key == '3') difficulty = "Hard";
                    break;

                case 1: 
                    werase(win);
                    mvwprintw(win, 1, 2, "Select Color Theme:");
                    mvwprintw(win, 3, 2, "1. Light");
                    mvwprintw(win, 4, 2, "2. Dark");
                    wrefresh(win);
                    key = wgetch(win);
                    if (key == '1') theme = "Light";
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
            break;
        case 1:
            mvwprintw(win, 3, 2, "Continuing the Game...");
            break;
        case 2:
            mvwprintw(win, 3, 2, "Displaying Leaderboard...");
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
            loggedInUser.email[0] = '\0'; // ایمیل خالی برای مهمان
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
    FILE* file = fopen("users.dat", "r");
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
    FILE* file = fopen("users.dat", "a");
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

    int height = 30, width = 100, start_y = 5, start_x = 15;
    WINDOW *menu_win = newwin(height, width, start_y, start_x);
    keypad(menu_win, TRUE);

    while (1) {
        dynamicMenu(menu_win, menu, n_options);
    }

    delwin(menu_win);
    endwin();
    return 0;
}
