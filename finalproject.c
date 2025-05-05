/* finalproject.c - C card game project with AI players */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_PLAYERS 4
#define MAX_CARDS 100
#define HAND_SIZE 7
#define NAME_LENGTH 50

// CARD STRUCTURE
typedef struct card_s {
    char name; // '0'-'9', 'A', 'O', 'N', 'R'
    char color; // 'R', 'Y', 'G', 'B', 'S'
} card;

typedef struct player_s {
    char name[NAME_LENGTH];
    card hand[MAX_CARDS];
    int handSize;
    int isAI;
} player;

// FUNCTION DECLARATIONS
void initializeDeck(card deck[], int *deckSize);
void shuffleDeck(card deck[], int deckSize);
int drawCard(card deck[], int *deckSize, player *p);
int isValidCard(card topOfPile, card candidate);
int handleAND(player *nextPlayer, card challenge);
int handleOR(player *nextPlayer, card challenge);
int handleNOT(int currentPlayerIndex, int playerCount);
void handleReverse(int *direction);
void printCard(card c);
void printPlayerHand(player p);
int aiChooseCard(player *ai, card topCard);

// MAIN FUNCTION
int main() {
    srand(time(NULL));
    
    card deck[MAX_CARDS];
    int deckSize;
    initializeDeck(deck, &deckSize);
    shuffleDeck(deck, deckSize);

    int numPlayers;
    player players[MAX_PLAYERS];

    printf("Enter number of players: ");
    scanf("%d", &numPlayers);
    getchar(); // consume newline

    for (int i = 0; i < numPlayers; i++) {
        printf("Is player %d an AI? (1 for yes, 0 for no): ", i+1);
        scanf("%d", &players[i].isAI);
        getchar();
        if (!players[i].isAI) {
            printf("Enter player %d's name: ", i + 1);
            fgets(players[i].name, NAME_LENGTH, stdin);
            players[i].name[strcspn(players[i].name, "\n")] = 0;
        } else {
            sprintf(players[i].name, "AI Player %d", i + 1);
        }
        players[i].handSize = 0;
        for (int j = 0; j < HAND_SIZE; j++) {
            drawCard(deck, &deckSize, &players[i]);
        }
    }

    for (int i = 0; i < numPlayers; i++) {
        printf("%s’s hand:\n", players[i].name);
        printPlayerHand(players[i]);
    }

    printf("Card pile is empty.\n");

    card pileTop = {'0', 'R'}; // Dummy starter card
    int currentPlayer = 0;
    int direction = 1; // 1 for forward, -1 for reverse
    int winner = -1;

    while (winner == -1 && deckSize > 0) {
        player *p = &players[currentPlayer];
        printf("%s's turn:\n", p->name);
        printPlayerHand(*p);
        printf("Top of pile: ");
        printCard(pileTop);

        int choice = -1;
        if (p->isAI) {
            choice = aiChooseCard(p, pileTop);
            printf("AI chose card %d\n", choice);
        } else {
            printf("Enter which card to play from 0 to %d: ", p->handSize - 1);
            scanf("%d", &choice);
        }

        if (choice >= 0 && choice < p->handSize && isValidCard(pileTop, p->hand[choice])) {
            card played = p->hand[choice];
            pileTop = played;
            for (int i = choice; i < p->handSize - 1; i++) {
                p->hand[i] = p->hand[i + 1];
            }
            p->handSize--;
            printf("%s played ", p->name); printCard(played);

            if (played.name == 'A') handleAND(&players[(currentPlayer + direction + numPlayers) % numPlayers], played);
            else if (played.name == 'O') handleOR(&players[(currentPlayer + direction + numPlayers) % numPlayers], played);
            else if (played.name == 'N') currentPlayer = handleNOT(currentPlayer, numPlayers);
            else if (played.name == 'R') handleReverse(&direction);

            if (p->handSize == 0) winner = currentPlayer;
        } else {
            printf("Invalid choice, drawing a card.\n");
            drawCard(deck, &deckSize, p);
        }

        if (winner == -1)
            currentPlayer = (currentPlayer + direction + numPlayers) % numPlayers;
    }

    printf("%s wins the game!\n", players[winner].name);
    return 0;
}

// FUNCTION IMPLEMENTATIONS
void initializeDeck(card deck[], int *deckSize) {
    const char colors[] = {'R', 'Y', 'G', 'B'};
    int index = 0;
    for (int c = 0; c < 4; c++) {
        for (int n = 0; n <= 9; n++) {
            for (int i = 0; i < 2; i++) {
                deck[index].name = '0' + n;
                deck[index].color = colors[c];
                index++;
            }
        }
    }
    for (int i = 0; i < 5; i++) {
        deck[index++] = (card){'A', 'S'};
        deck[index++] = (card){'O', 'S'};
        deck[index++] = (card){'N', 'S'};
        deck[index++] = (card){'R', 'S'};
    }
    *deckSize = index;
}

void shuffleDeck(card deck[], int deckSize) {
    for (int i = 0; i < 10000; i++) {
        int a = rand() % deckSize;
        int b = rand() % deckSize;
        card tmp = deck[a];
        deck[a] = deck[b];
        deck[b] = tmp;
    }
}

int drawCard(card deck[], int *deckSize, player *p) {
    if (*deckSize <= 0) return 0;
    p->hand[p->handSize++] = deck[--(*deckSize)];
    return 1;
}

int isValidCard(card topOfPile, card candidate) {
    if (candidate.color == 'S') return 1;
    return candidate.color == topOfPile.color || candidate.name == topOfPile.name;
}

int handleAND(player *nextPlayer, card challenge) {
    for (int i = 0; i < nextPlayer->handSize; i++) {
        if (nextPlayer->hand[i].color == challenge.color && nextPlayer->hand[i].name == challenge.name) return 1;
    }
    for (int i = 0; i < 4; i++) drawCard(NULL, NULL, nextPlayer); // simulate draw
    return 0;
}

int handleOR(player *nextPlayer, card challenge) {
    for (int i = 0; i < nextPlayer->handSize; i++) {
        if (nextPlayer->hand[i].color == challenge.color || nextPlayer->hand[i].name == challenge.name) return 1;
    }
    for (int i = 0; i < 4; i++) drawCard(NULL, NULL, nextPlayer); // simulate draw
    return 0;
}

int handleNOT(int currentPlayerIndex, int playerCount) {
    return currentPlayerIndex; // skip other player in 2P, stay on current
}

void handleReverse(int *direction) {
    *direction *= -1;
}

void printCard(card c) {
    if (c.color == 'S') {
        if (c.name == 'A') printf("AND\n");
        else if (c.name == 'O') printf("OR\n");
        else if (c.name == 'N') printf("NOT\n");
        else if (c.name == 'R') printf("Reverse\n");
    } else {
        char *color;
        if (c.color == 'R') color = "Red";
        else if (c.color == 'Y') color = "Yellow";
        else if (c.color == 'G') color = "Green";
        else color = "Blue";
        printf("%s %c\n", color, c.name);
    }
}

void printPlayerHand(player p) {
    for (int i = 0; i < p.handSize; i++) {
        printf("%d: ", i);
        printCard(p.hand[i]);
    }
}

int aiChooseCard(player *ai, card topCard) {
    for (int i = 0; i < ai->handSize; i++) {
        if (isValidCard(topCard, ai->hand[i])) return i;
    }
    return -1;
}

