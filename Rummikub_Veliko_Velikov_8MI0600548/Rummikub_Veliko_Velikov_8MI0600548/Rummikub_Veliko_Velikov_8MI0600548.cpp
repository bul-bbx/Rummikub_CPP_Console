#include <iostream>
#include <cstdlib>
#include <sstream>
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
    else cout << "\033[35m";
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
    for (int k = 0; k < 2; k++) for (int c = 0; c < 4; c++) for (int n = 1; n <= 13; n++) {
        deck[deckSize].number = n; deck[deckSize].color = c; deckSize++;
    }
    for (int i = 0; i < 2; i++) { deck[deckSize].number = 0; deck[deckSize].color = JOKER; deckSize++; }
}
void shuffleDeck() { for (int i = 0; i < deckSize; i++) { int j = rand() % deckSize; Tile t = deck[i]; deck[i] = deck[j]; deck[j] = t; } }

// ---------------- HAND ----------------
void sortHand(Player& p) {
    for (int i = 0; i < p.handSize - 1; i++) for (int j = i + 1; j < p.handSize; j++)
        if (p.hand[i].number > p.hand[j].number || (p.hand[i].number == p.hand[j].number && p.hand[i].color > p.hand[j].color)) {
            Tile t = p.hand[i]; p.hand[i] = p.hand[j]; p.hand[j] = t;
        }
}
void drawTile(Player& p) { if (deckSize > 0 && p.handSize < MAX_HAND) { p.hand[p.handSize] = deck[deckSize - 1]; p.handSize++; deckSize--; sortHand(p); } }
void removeTile(Player& p, int index) { for (int i = index; i < p.handSize - 1; i++) p.hand[i] = p.hand[i + 1]; p.handSize--; }
void printHand(Player p) { for (int i = 0; i < p.handSize; i++) { printTile(p.hand[i]); cout << " "; } cout << endl; }

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
void addGroup(Tile group[], int size) { for (int i = 0; i < size; i++) board[boardSize][i] = group[i]; groupSizes[boardSize] = size; boardSize++; }
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
        if (group[i].color == JOKER) continue; if (number == -1) number = group[i].number; else if (group[i].number != number) return false;
        if (usedColor[group[i].color]) return false; usedColor[group[i].color] = true;
    } return true;
}

bool isRun(Tile group[], int size) {
    int color = -1, numbers[13], count = 0;
    for (int i = 0; i < size; i++) { if (group[i].color != JOKER) { if (color == -1) color = group[i].color; else if (group[i].color != color) return false; numbers[count++] = group[i].number; } }
    for (int i = 0; i < count - 1; i++) for (int j = i + 1; j < count; j++) if (numbers[i] > numbers[j]) { int t = numbers[i]; numbers[i] = numbers[j]; numbers[j] = t; }
    for (int i = 0; i < count - 1; i++) if (numbers[i + 1] != numbers[i] + 1) return false; return true;
}

// ---------------- SORT RUN ----------------
void sortRun(Tile group[], int size) {
    for (int i = 0; i < size - 1; i++) {
        for (int j = i + 1; j < size; j++) {
            if (group[i].color != JOKER && group[j].color != JOKER && group[i].number > group[j].number) {
                Tile t = group[i]; group[i] = group[j]; group[j] = t;
            }
        }
    }
}

// ---------------- PARSE TILE ----------------
bool parseTile(string s, Tile& t) {
    if (s == "J" || s == "j") { t.number = 0; t.color = JOKER; return true; }
    if (s.length() < 2) return false;
    int num = 0, i = 0;
    if (isdigit(s[0]) && isdigit(s[1])) { num = (s[0] - '0') * 10 + (s[1] - '0'); i = 2; }
    else { num = s[0] - '0'; i = 1; }
    if (num < 1 || num>13) return false;
    char c = s[i]; int color = -2;
    if (c == 'R' || c == 'r') color = RED; else if (c == 'B' || c == 'b') color = BLUE;
    else if (c == 'K' || c == 'k') color = BLACK; else if (c == 'Y' || c == 'y') color = YELLOW; else return false;
    t.number = num; t.color = color; return true;
}

// ---------------- FIND TILE ----------------
int findTileInHand(Player& p, Tile t) { for (int i = 0; i < p.handSize; i++) if (p.hand[i].number == t.number && p.hand[i].color == t.color) return i; return -1; }
int findTileInGroup(int g, Tile t) { for (int i = 0; i < groupSizes[g]; i++) if (board[g][i].number == t.number && board[g][i].color == t.color) return i; return -1; }

// ---------------- MAIN ----------------
int main() {
    srand(1);
    int numPlayers; cout << "Number of players (2-4): "; cin >> numPlayers; if (numPlayers < 2) numPlayers = 2; if (numPlayers > 4) numPlayers = 4;
    Player players[4]; for (int i = 0; i < numPlayers; i++) players[i].handSize = 0;
    createDeck(); shuffleDeck(); for (int i = 0; i < 14; i++) for (int j = 0; j < numPlayers; j++) drawTile(players[j]);

    int currentPlayer = 0; bool running = true;
    while (running) {
        system("cls");
        cout << "PLAYER " << (currentPlayer + 1) << "'S TURN\n";
        printBoard(); cout << "\nYour hand:\n"; printHand(players[currentPlayer]);
        Player& p = players[currentPlayer]; bool turnDone = false;

        while (!turnDone) {
            cout << "\nOptions:\n1. Draw tile\n2. New group\n3. Add tiles to group\n4. Move tiles from board to new group\n5. End turn\nChoice: ";
            int choice; cin >> choice; cin.ignore();

            if (choice == 1) { drawTile(p); cout << "Drew a tile.\n"; }
            else if (choice == 2) {
                cout << "Enter tiles for new group: "; string line; getline(cin, line); stringstream ss(line); string s; Tile group[13]; int k = 0;
                while (ss >> s) { Tile t; if (!parseTile(s, t)) { cout << "Invalid: " << s << endl; k = 0; break; } int idx = findTileInHand(p, t); if (idx == -1) { cout << "Tile not in hand: " << s << endl; k = 0; break; } group[k++] = t; }
                if (k == 0) continue;
                if (isSet(group, k) || isRun(group, k)) {
                    if (isRun(group, k)) sortRun(group, k);
                    addGroup(group, k);
                    for (int i = 0; i < k; i++) { int idx = findTileInHand(p, group[i]); removeTile(p, idx); }
                    sortHand(p); cout << "Group placed!\n";
                }
                else cout << "Invalid group.\n";
            }
            else if (choice == 3) {
                cout << "Enter board group number: "; int g; cin >> g; cin.ignore(); if (g<1 || g>boardSize) { cout << "Invalid group.\n"; continue; } g--;
                cout << "Enter tiles to add from your hand: "; string line; getline(cin, line); stringstream ss(line); string s; Tile groupAdd[13]; int k = 0;
                while (ss >> s) { Tile t; if (!parseTile(s, t)) { cout << "Invalid: " << s << endl; k = 0; break; } int idx = findTileInHand(p, t); if (idx == -1) { cout << "Tile not in hand: " << s << endl; k = 0; break; } groupAdd[k++] = t; }
                if (k == 0) continue;
                Tile temp[13]; int sizeNew = groupSizes[g]; for (int i = 0; i < sizeNew; i++) temp[i] = board[g][i]; for (int i = 0; i < k; i++) temp[sizeNew + i] = groupAdd[i];
                if (isSet(temp, sizeNew + k) || isRun(temp, sizeNew + k)) {
                    if (isRun(temp, sizeNew + k)) sortRun(temp, sizeNew + k);
                    for (int i = 0; i < k; i++) { int idx = findTileInHand(p, groupAdd[i]); removeTile(p, idx); }
                    for (int i = 0; i < sizeNew + k; i++) board[g][i] = temp[i];
                    groupSizes[g] = sizeNew + k;
                    sortHand(p); cout << "Added tiles!\n";
                }
                else cout << "Invalid addition.\n";
            }
            else if (choice == 4) {
                cout << "Enter board group number to take tiles from: "; int g; cin >> g; cin.ignore(); if (g<1 || g>boardSize) { cout << "Invalid group.\n"; continue; } g--;
                cout << "Enter tiles to take: "; string line; getline(cin, line); stringstream ss(line); string s; Tile taken[13]; int k = 0;
                while (ss >> s) { Tile t; if (!parseTile(s, t)) { cout << "Invalid: " << s << endl; k = 0; break; } int idx = findTileInGroup(g, t); if (idx == -1) { cout << "Tile not in group: " << s << endl; k = 0; break; } taken[k++] = t; }
                if (k == 0) continue;
                for (int i = 0; i < k; i++) { int idx = findTileInGroup(g, taken[i]); removeTileFromGroup(g, idx); }
                cout << "Optionally add tiles from your hand: "; string handLine; getline(cin, handLine); stringstream ss2(handLine); string tstr; Tile groupNew[13]; int m = 0;
                for (int i = 0; i < k; i++) groupNew[m++] = taken[i];
                while (ss2 >> tstr) { Tile t; if (!parseTile(tstr, t)) { cout << "Invalid: " << tstr << endl; continue; } int idx = findTileInHand(p, t); if (idx == -1) { cout << "Tile not in hand: " << tstr << endl; continue; } groupNew[m++] = t; removeTile(p, idx); }
                if (isSet(groupNew, m) || isRun(groupNew, m)) {
                    if (isRun(groupNew, m)) sortRun(groupNew, m);
                    addGroup(groupNew, m); cout << "New group created!\n";
                }
                else { cout << "Invalid group! Returning tiles.\n"; for (int i = 0; i < k; i++) { addGroup(&taken[i], 1); } }
            }
            else if (choice == 5) turnDone = true;
            else cout << "Invalid choice.\n";

            if (p.handSize == 0) { cout << "\nPLAYER " << (currentPlayer + 1) << " WINS!\n"; running = false; break; }
            printBoard(); cout << "\nYour hand:\n"; printHand(p);
        }
        if (!running) break;
        currentPlayer = (currentPlayer + 1) % numPlayers;
    }

    cout << "\nGame Over.\n";
    return 0;
}
