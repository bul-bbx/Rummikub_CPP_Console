#include <iostream>
#include <cstdlib>
using namespace std;

const int MAX_DECK = 110, MAX_HAND = 30, MAX_GROUPS = 50;
const int RED = 0, BLUE = 1, BLACK = 2, YELLOW = 3, JOKER = -1;

struct Tile { int number; int color; };
struct Player { Tile hand[MAX_HAND]; int handSize; };

Tile deck[MAX_DECK]; int deckSize = 0;
Tile board[MAX_GROUPS][13]; int groupSizes[MAX_GROUPS]; int boardSize = 0;

// ---------------- COLORS ----------------
void setColor(int color) {
    if (color == RED) cout << "\033[31m";
    else if (color == BLUE) cout << "\033[34m";
    else if (color == BLACK) cout << "\033[37m";
    else if (color == YELLOW) cout << "\033[33m";
    else cout << "\033[35m"; // Joker
}
void resetColor() { cout << "\033[0m"; }

// ---------------- TILE ----------------
void printTile(Tile t) {
    if (t.color == JOKER) { setColor(JOKER); cout << "J"; resetColor(); return; }
    setColor(t.color); cout << t.number;
    if (t.color == RED) cout << "R"; else if (t.color == BLUE) cout << "B";
    else if (t.color == BLACK) cout << "K"; else if (t.color == YELLOW) cout << "Y";
    resetColor();
}

// ---------------- DECK ----------------
void createDeck() {
    deckSize = 0;
    for (int k = 0; k < 2; k++)
        for (int c = 0; c < 4; c++)
            for (int n = 1; n <= 13; n++) {
                deck[deckSize].number = n;
                deck[deckSize].color = c;
                deckSize++;
            }
    for (int i = 0; i < 2; i++) { deck[deckSize].number = 0; deck[deckSize].color = JOKER; deckSize++; }
}

void shuffleDeck() {
    for (int i = 0; i < deckSize; i++) {
        int j = rand() % deckSize;
        Tile t = deck[i]; deck[i] = deck[j]; deck[j] = t;
    }
}

// ---------------- HAND ----------------
void sortHand(Player& p) {
    for (int i = 0; i < p.handSize - 1; i++)
        for (int j = i + 1; j < p.handSize; j++)
            if (p.hand[i].number > p.hand[j].number ||
                (p.hand[i].number == p.hand[j].number && p.hand[i].color > p.hand[j].color)) {
                Tile t = p.hand[i]; p.hand[i] = p.hand[j]; p.hand[j] = t;
            }
}

void drawTile(Player& p) {
    if (deckSize > 0 && p.handSize < MAX_HAND) {
        p.hand[p.handSize] = deck[deckSize - 1];
        p.handSize++;
        deckSize--;
        sortHand(p);
    }
}

void removeTile(Player& p, int index) {
    for (int i = index; i < p.handSize - 1; i++) p.hand[i] = p.hand[i + 1];
    p.handSize--;
}

void printHand(Player p) {
    for (int i = 0; i < p.handSize; i++) { printTile(p.hand[i]); cout << " "; }
    cout << endl;
}

// ---------------- BOARD ----------------
void printBoard() {
    cout << "\n=========== BOARD ===========\n";
    for (int i = 0; i < boardSize; i++) {
        cout << "Group " << i + 1 << ": ";
        for (int j = 0; j < groupSizes[i]; j++) { printTile(board[i][j]); cout << " "; }
        cout << endl;
    }
    cout << "=============================\n";
}

void addGroup(Tile group[], int size) {
    for (int i = 0; i < size; i++) board[boardSize][i] = group[i];
    groupSizes[boardSize] = size;
    boardSize++;
}

void removeTileFromGroup(int groupIndex, int tileIndex) {
    for (int i = tileIndex; i < groupSizes[groupIndex] - 1; i++) board[groupIndex][i] = board[groupIndex][i + 1];
    groupSizes[groupIndex]--;
    if (groupSizes[groupIndex] == 0) {
        for (int i = groupIndex; i < boardSize - 1; i++) {
            groupSizes[i] = groupSizes[i + 1];
            for (int j = 0; j < groupSizes[i]; j++) board[i][j] = board[i + 1][j];
        }
        boardSize--;
    }
}

// ---------------- GROUP CHECK ----------------
bool isSet(Tile group[], int size) {
    int number = -1; bool usedColor[4] = { false,false,false,false };
    for (int i = 0; i < size; i++) {
        if (group[i].color == JOKER) continue;
        if (number == -1) number = group[i].number;
        else if (group[i].number != number) return false;
        if (usedColor[group[i].color]) return false;
        usedColor[group[i].color] = true;
    }
    return size >= 3;
}

bool isRun(Tile group[], int size) {
    if (size < 3) return false;
    int color = -1;
    int numbers[13]; int count = 0; int jokers = 0;
    for (int i = 0; i < size; i++) {
        if (group[i].color == JOKER) { jokers++; continue; }
        if (color == -1) color = group[i].color;
        else if (group[i].color != color) return false;
        numbers[count++] = group[i].number;
    }
    // Sort numbers
    for (int i = 0; i < count - 1; i++)
        for (int j = i + 1; j < count; j++)
            if (numbers[i] > numbers[j]) { int t = numbers[i]; numbers[i] = numbers[j]; numbers[j] = t; }

    // Check gaps using jokers
    for (int i = 0; i < count - 1; i++) {
        int gap = numbers[i + 1] - numbers[i] - 1;
        if (gap < 0) return false; // duplicate
        jokers -= gap;
        if (jokers < 0) return false;
    }
    return true;
}

void sortRun(Tile group[], int size) {
    Tile result[13]; int color = -1; int numbers[13]; int count = 0; int jokers = 0;
    for (int i = 0; i < size; i++) {
        if (group[i].color == JOKER) jokers++;
        else {
            if (color == -1) color = group[i].color;
            numbers[count++] = group[i].number;
        }
    }
    // Sort numbers
    for (int i = 0; i < count - 1; i++)
        for (int j = i + 1; j < count; j++)
            if (numbers[i] > numbers[j]) { int t = numbers[i]; numbers[i] = numbers[j]; numbers[j] = t; }

    // Build run with jokers filling gaps
    int idx = 0, current = numbers[0];
    result[idx].number = current; result[idx].color = color; idx++;
    for (int i = 1; i < count; i++) {
        while (current + 1 < numbers[i] && jokers > 0) {
            result[idx].number = 0; result[idx].color = JOKER; idx++; jokers--; current++;
        }
        result[idx].number = numbers[i]; result[idx].color = color; idx++; current = numbers[i];
    }
    while (jokers-- > 0) { result[idx].number = 0; result[idx].color = JOKER; idx++; }
    for (int i = 0; i < size; i++) group[i] = result[i];
}

// ---------------- PARSE TILE ----------------
bool parseTile(char s[], Tile& t) {
    if (s[0] == 'J' || s[0] == 'j') { t.number = 0; t.color = JOKER; return true; }
    int num = 0;
    int i = 0;
    if (s[0] >= '0' && s[0] <= '9' && s[1] >= '0' && s[1] <= '9') { num = (s[0] - '0') * 10 + (s[1] - '0'); i = 2; }
    else { num = s[0] - '0'; i = 1; }
    if (num < 1 || num > 13) return false;
    char c = s[i]; int color = -2;
    if (c == 'R' || c == 'r') color = RED;
    else if (c == 'B' || c == 'b') color = BLUE;
    else if (c == 'K' || c == 'k') color = BLACK;
    else if (c == 'Y' || c == 'y') color = YELLOW;
    else return false;
    t.number = num; t.color = color; return true;
}

// ---------------- FIND TILE ----------------
int findTileInHand(Player& p, Tile t) { for (int i = 0; i < p.handSize; i++) if (p.hand[i].number == t.number && p.hand[i].color == t.color) return i; return -1; }
int findTileInGroup(int g, Tile t) { for (int i = 0; i < groupSizes[g]; i++) if (board[g][i].number == t.number && board[g][i].color == t.color) return i; return -1; }

// ---------------- POINT CALCULATION ----------------
int calculateHandPoints(Player& p) {
    int score = 0;
    for (int i = 0; i < p.handSize; i++) {
        if (p.hand[i].color == JOKER) score += 30;
        else score += p.hand[i].number;
    }
    return score;
}

// ---------------- CHECK DECK EMPTY ----------------
bool checkDeckEmptyEndGame(Player players[], int numPlayers) {
    if (deckSize > 0) return false;
    cout << "\nThe deck is empty! Calculating scores...\n";
    int minPoints = 9999, winner = -1;
    for (int i = 0; i < numPlayers; i++) {
        int points = calculateHandPoints(players[i]);
        cout << "Player " << (i + 1) << " has " << points << " points left.\n";
        if (points < minPoints) { minPoints = points; winner = i; }
    }
    cout << "\nPLAYER " << (winner + 1) << " WINS with the lowest points (" << minPoints << ")!\n";
    return true;
}

// ---------------- MAIN ----------------
int main() {
    srand(time(0));
    int numPlayers; cout << "Number of players (2-4): "; cin >> numPlayers; cin.ignore();
    if (numPlayers < 2) numPlayers = 2;
    if (numPlayers > 4) numPlayers = 4;

    Player players[4]; for (int i = 0; i < numPlayers; i++) players[i].handSize = 0;
    bool hasOpened[4]; for (int i = 0; i < numPlayers; i++) hasOpened[i] = false;

    createDeck(); shuffleDeck();
    for (int i = 0; i < 14; i++) for (int j = 0; j < numPlayers; j++) drawTile(players[j]);

    int currentPlayer = 0;
    bool running = true;

    while (running) {

    restart_turn:
        system("cls");
        cout << "PLAYER " << (currentPlayer + 1) << "'S TURN\n";
        printBoard();
        cout << "\nYour hand:\n"; printHand(players[currentPlayer]);
        Player& p = players[currentPlayer];
        bool turnDone = false;
        bool madeMove = false;

        // Backup board/hand
        Tile boardBackup[MAX_GROUPS][13]; int groupSizesBackup[MAX_GROUPS]; int boardSizeBackup = boardSize;
        for (int i = 0; i < boardSize; i++) { groupSizesBackup[i] = groupSizes[i]; for (int j = 0; j < groupSizes[i]; j++) boardBackup[i][j] = board[i][j]; }
        Tile handBackup[MAX_HAND]; int handSizeBackup = p.handSize;
        for (int i = 0; i < p.handSize; i++) handBackup[i] = p.hand[i];
        int turnPoints = 0;

        while (!turnDone) {
            system("cls");
            cout << "PLAYER " << (currentPlayer + 1) << " - PLAY MODE\n";
            printBoard(); cout << "\nYour hand:\n"; printHand(p); cout << "Tiles left in deck: " << deckSize << endl;
            if (!hasOpened[currentPlayer]) cout << "\n(Initial meld: need at least 30 points, current: " << turnPoints << ")\n";

            int choice = 0;
            char choiceLine[51];
            while (true) {
                cout << "\nPlay Options:\n";
                cout << "1. New group\n";
                if (hasOpened[currentPlayer]) {
                    cout << "2. Add tiles to group\n3. Move tiles from board to new group\n";
                }
                else {
                    cout << "2. (Locked)\n3. (Locked)\n";
                }
                cout << "4. End turn\nChoice: ";

                cin.getline(choiceLine, 51);

                // Try to convert to integer
                bool valid = true;
                int tempChoice = 0;
                int i = 0;
                while (choiceLine[i] != '\0') {
                    if (choiceLine[i] >= '0' && choiceLine[i] <= '9') tempChoice = tempChoice * 10 + (choiceLine[i] - '0');
                    else if (choiceLine[i] == ' ' || choiceLine[i] == '\t') {}
                    else { valid = false; break; }
                    i++;
                }

                if (!valid) { cout << "Invalid choice. Try again.\n"; continue; }

                choice = tempChoice;
                if ((choice >= 1 && choice <= 4) && (hasOpened[currentPlayer] || choice == 1 || choice == 4)) break;
                cout << "Invalid choice. Try again.\n";
            }

            // --- NEW GROUP ---
            if (choice == 1) {
                char line[51]; cout << "Enter tiles for new group: "; cin.getline(line, 51);
                Tile group[13]; int k = 0;
                int i = 0;
                while (line[i] != '\0') {
                    while (line[i] == ' ') i++;
                    if (line[i] == '\0') break;
                    char tileStr[5]; int tIdx = 0;
                    while (line[i] != ' ' && line[i] != '\0') { tileStr[tIdx++] = line[i]; i++; }
                    tileStr[tIdx] = '\0';
                    Tile t;
                    if (!parseTile(tileStr, t)) { cout << "Invalid: " << tileStr << endl; k = 0; break; }
                    int idx = findTileInHand(p, t); if (idx == -1) { cout << "Tile not in hand: " << tileStr << endl; k = 0; break; }
                    group[k++] = t;
                }
                if (k == 0) continue;
                if (isSet(group, k) || isRun(group, k)) {
                    if (isRun(group, k)) sortRun(group, k); addGroup(group, k);
                    for (int i = 0; i < k; i++) { int idx = findTileInHand(p, group[i]); removeTile(p, idx); if (group[i].color != JOKER) turnPoints += group[i].number; }
                    sortHand(p); madeMove = true; cout << "Group placed!\n"; system("pause");
                }
                else { cout << "Invalid group.\n"; system("pause"); }
            }

            // --- ADD TO GROUP ---
            else if (choice == 2) {
                if (!hasOpened[currentPlayer]) { cout << "Cannot add tiles before opening meld.\n"; system("pause"); continue; }
                cout << "Enter board group number: "; int g; cin >> g; cin.ignore();
                if (g<1 || g>boardSize) { cout << "Invalid group.\n"; system("pause"); continue; } g--;
                char line[51]; cout << "Enter tiles to add from your hand: "; cin.getline(line, 51);
                Tile groupAdd[13]; int k = 0; int i = 0;
                while (line[i] != '\0') {
                    while (line[i] == ' ') i++; if (line[i] == '\0') break;
                    char tileStr[5]; int tIdx = 0;
                    while (line[i] != ' ' && line[i] != '\0') { tileStr[tIdx++] = line[i]; i++; }
                    tileStr[tIdx] = '\0';
                    Tile t; if (!parseTile(tileStr, t)) { cout << "Invalid: " << tileStr << endl; k = 0; break; }
                    int idx = findTileInHand(p, t); if (idx == -1) { cout << "Tile not in hand: " << tileStr << endl; k = 0; break; }
                    groupAdd[k++] = t;
                }
                if (k == 0) continue;
                Tile temp[13]; int sizeNew = groupSizes[g]; for (int i = 0; i < sizeNew; i++) temp[i] = board[g][i]; for (int i = 0; i < k; i++) temp[sizeNew + i] = groupAdd[i];
                if (isSet(temp, sizeNew + k) || isRun(temp, sizeNew + k)) {
                    if (isRun(temp, sizeNew + k)) sortRun(temp, sizeNew + k);
                    for (int i = 0; i < k; i++) { int idx = findTileInHand(p, groupAdd[i]); removeTile(p, idx); if (groupAdd[i].color != JOKER) turnPoints += groupAdd[i].number; }
                    for (int i = 0; i < sizeNew + k; i++) board[g][i] = temp[i]; groupSizes[g] = sizeNew + k; sortHand(p); madeMove = true; cout << "Added tiles!\n"; system("pause");
                }
                else { cout << "Invalid addition.\n"; system("pause"); }
            }

            // --- MOVE TILES FROM BOARD ---
            else if (choice == 3) {
                if (!hasOpened[currentPlayer]) { cout << "Cannot move tiles before opening meld.\n"; system("pause"); continue; }
                cout << "Enter board group number to take tiles from: "; int g; cin >> g; cin.ignore();
                if (g<1 || g>boardSize) { cout << "Invalid group.\n"; system("pause"); continue; } g--;
                char line[51]; cout << "Enter tiles to take: "; cin.getline(line, 51);
                Tile taken[13]; int k = 0; int i = 0;
                while (line[i] != '\0') {
                    while (line[i] == ' ') i++; if (line[i] == '\0') break; char tileStr[5]; int tIdx = 0;
                    while (line[i] != ' ' && line[i] != '\0') { tileStr[tIdx++] = line[i]; i++; }
                    tileStr[tIdx] = '\0'; Tile t; if (!parseTile(tileStr, t)) { cout << "Invalid: " << tileStr << endl; k = 0; break; }
                    int idx = findTileInGroup(g, t); if (idx == -1) { cout << "Tile not in group: " << tileStr << endl; k = 0; break; } taken[k++] = t;
                }
                if (k == 0) continue;
                for (int i = 0; i < k; i++) { int idx = findTileInGroup(g, taken[i]); removeTileFromGroup(g, idx); }
                char handLine[51]; cout << "Optionally add tiles from your hand: "; cin.getline(handLine, 51);
                Tile groupNew[13]; int m = 0; for (int i = 0; i < k; i++) groupNew[m++] = taken[i];
                i = 0; while (handLine[i] != '\0') {
                    while (handLine[i] == ' ') i++; if (handLine[i] == '\0') break;
                    char tileStr[5]; int tIdx = 0; while (handLine[i] != ' ' && handLine[i] != '\0') { tileStr[tIdx++] = handLine[i]; i++; }
                    tileStr[tIdx] = '\0'; Tile t; if (!parseTile(tileStr, t)) { cout << "Invalid: " << tileStr << endl; continue; }
                    int idx = findTileInHand(p, t); if (idx == -1) { cout << "Tile not in hand: " << tileStr << endl; continue; }
                    groupNew[m++] = t; removeTile(p, idx); if (t.color != JOKER) turnPoints += t.number;
                }
                if (isSet(groupNew, m) || isRun(groupNew, m)) { if (isRun(groupNew, m)) sortRun(groupNew, m); addGroup(groupNew, m); madeMove = true; cout << "New group created!\n"; system("pause"); }
                else {
                    cout << "Invalid group! Returning tiles.\n"; system("pause");
                    // Restore board
                    boardSize = boardSizeBackup; for (int i = 0; i < boardSize; i++) { groupSizes[i] = groupSizesBackup[i]; for (int j = 0; j < groupSizes[i]; j++) board[i][j] = boardBackup[i][j]; }
                    p.handSize = handSizeBackup; for (int i = 0; i < p.handSize; i++) p.hand[i] = handBackup[i]; turnPoints = 0; goto restart_turn;
                }
            }

            // --- END TURN ---
            else if (choice == 4) {
                if (!madeMove) { cout << "\nYou did not make any move. You must draw a tile.\n"; drawTile(p); system("pause"); turnDone = true; break; }
                if (!hasOpened[currentPlayer] && turnPoints < 30) {
                    cout << "\nInitial meld must be at least 30 points! You played " << turnPoints << " points.\nBoard reset.\n"; system("pause");
                    boardSize = boardSizeBackup; for (int i = 0; i < boardSize; i++) { groupSizes[i] = groupSizesBackup[i]; for (int j = 0; j < groupSizes[i]; j++) board[i][j] = boardBackup[i][j]; }
                    p.handSize = handSizeBackup; for (int i = 0; i < p.handSize; i++) p.hand[i] = handBackup[i]; turnPoints = 0; goto restart_turn;
                }
                hasOpened[currentPlayer] = true; turnDone = true;
            }

            if (p.handSize == 0) { cout << "\nPLAYER " << (currentPlayer + 1) << " WINS!\n"; running = false; break; }
        }

        if (!running) break;
        currentPlayer = (currentPlayer + 1) % numPlayers;
        if (checkDeckEmptyEndGame(players, numPlayers)) { running = false; break; }
    }

    cout << "\nGame Over.\n";
    return 0;
}
