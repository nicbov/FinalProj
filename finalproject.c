#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <ctype.h>

typedef struct card_s {
    char name;
    char color;
    struct card_s *next;
} card;

typedef struct {
    char name[50];
    card *hand;
    int hand_size;
    bool is_ai;
} player;

void initializeDeck(card **deck, int *deckSize);
void shuffleDeck(card **deck, int *deckSize);
int drawCard(card **deck, int *deckSize, player *p);
int isValidCard(card topOfPile, card candidate, char currentColor);
int handleAND(player *currentPlayer, card topOfPile, card **deck, int *deckSize, player *nextPlayer, char *currentColor, card **pile, int *pileSize);
int handleOR(player *currentPlayer, card topOfPile, card **deck, int *deckSize, player *nextPlayer, char *currentColor, card **pile, int *pileSize);
int handleNOT(int *currentPlayerIndex, int playerCount, int *direction);
void handleReverse(int *currentPlayerIndex, int playerCount, int *direction);
void printCard(card c);
void printPlayerHand(player p);
void freeHand(card *hand);
void addCardToHand(card **hand, card newCard, int *hand_size);
void removeCardFromHand(card **hand, int index, int *hand_size);
void aiPlay(player *ai, card topOfPile, card **pile, card **deck, int *deckSize, int *currentPlayerIndex, int playerCount, player *players, int *direction, char *currentColor, int *pileSize);
void playCard(player *currentPlayer, int cardIndex, card **pile, card **deck, int *deckSize, int *currentPlayerIndex, int playerCount, player *players, int *direction, char *currentColor, int *pileSize);
char getColorName(char colorCode);

void initializeDeck(card **deck, int *deckSize) {
    char colors[] = {'R', 'Y', 'G', 'B'};
    char numbers[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
    char specials[] = {'A', 'O', 'N', 'R'};
    
    for (int c = 0; c < 4; c++) {
        for (int n = 0; n < 10; n++) {
            for (int i = 0; i < 2; i++) {
                card newCard = {numbers[n], colors[c], NULL};
                addCardToHand(deck, newCard, deckSize);
            }
        }
    }
    
    for (int s = 0; s < 4; s++) {
        for (int i = 0; i < 5; i++) {
            card newCard = {specials[s], 'S', NULL};
            addCardToHand(deck, newCard, deckSize);
        }
    }
}

void shuffleDeck(card **deck, int *deckSize) {
    for (int i = 0; i < 10000; i++) {
        int index1 = rand() % *deckSize;
        int index2 = rand() % *deckSize;
        
        card temp = (*deck)[index1];
        (*deck)[index1] = (*deck)[index2];
        (*deck)[index2] = temp;
    }
}

int drawCard(card **deck, int *deckSize, player *p) {
    if (*deckSize == 0) return 0;
    
    card drawnCard = (*deck)[0];
    removeCardFromHand(deck, 0, deckSize);
    addCardToHand(&(p->hand), drawnCard, &(p->hand_size));
    return 1;
}

int isValidCard(card topOfPile, card candidate, char currentColor) {
    if (candidate.color == 'S') return 1;
    return (candidate.color == currentColor) || (candidate.name == topOfPile.name);
}

int handleAND(player *currentPlayer, card topOfPile, card **deck, int *deckSize, player *nextPlayer, char *currentColor, card **pile, int *pileSize) {
    // Get the actual paired number card from the pile (second-to-last card)
    char pairedNumber = (*pile)[*pileSize-2].name;
    char pairedColor = (*pile)[*pileSize-2].color;

    printf("%s played AND with ", currentPlayer->name);
    printCard((*pile)[*pileSize-2]);
    printf(". %s must play a card matching BOTH color (%c) and number (%c) or draw 4.\n", 
           nextPlayer->name, getColorName(pairedColor), pairedNumber);
    
    bool hasMatch = false;
    for (int i = 0; i < nextPlayer->hand_size; i++) {
        card c = nextPlayer->hand[i];
        if (c.color == pairedColor && c.name == pairedNumber) {
            hasMatch = true;
            break;
        }
    }
    
    if (!hasMatch) {
        printf("%s doesn't have a matching card. Drawing 4 cards.\n", nextPlayer->name);
        for (int i = 0; i < 4; i++) {
            if (!drawCard(deck, deckSize, nextPlayer)) {
                printf("Deck is empty!\n");
                break;
            }
        }
        return 0;
    }
    
    for (int i = 0; i < nextPlayer->hand_size; i++) {
        card c = nextPlayer->hand[i];
        if (c.color == pairedColor && c.name == pairedNumber) {
            printf("%s plays matching card: ", nextPlayer->name);
            printCard(c);
            printf("\n");
            
            removeCardFromHand(&(nextPlayer->hand), i, &(nextPlayer->hand_size));
            addCardToHand(pile, c, pileSize);
            *currentColor = c.color;
            return 1;
        }
    }
    
    return 1;
}

int handleOR(player *currentPlayer, card topOfPile, card **deck, int *deckSize, player *nextPlayer, char *currentColor, card **pile, int *pileSize) {
    // Get the actual paired number card from the pile (second-to-last card)
    char pairedNumber = (*pile)[*pileSize-2].name;
    char pairedColor = (*pile)[*pileSize-2].color;

    printf("%s played OR with ", currentPlayer->name);
    printCard((*pile)[*pileSize-2]);
    printf(". %s must play a card matching EITHER color (%c) OR number (%c) or draw 4.\n",
           nextPlayer->name, getColorName(pairedColor), pairedNumber);
    
    bool hasMatch = false;
    int matchIndex = -1;
    for (int i = 0; i < nextPlayer->hand_size; i++) {
        card c = nextPlayer->hand[i];
        if (c.color == pairedColor || c.name == pairedNumber) {
            hasMatch = true;
            matchIndex = i;
            break;
        }
    }
    
    if (!hasMatch) {
        printf("%s doesn't have a matching card. Drawing 4 cards.\n", nextPlayer->name);
        for (int i = 0; i < 4; i++) {
            if (!drawCard(deck, deckSize, nextPlayer)) {
                printf("Deck is empty!\n");
                break;
            }
        }
        return 0;
    }
    
    if (matchIndex != -1) {
        card c = nextPlayer->hand[matchIndex];
        printf("%s plays matching card: ", nextPlayer->name);
        printCard(c);
        printf("\n");
        
        removeCardFromHand(&(nextPlayer->hand), matchIndex, &(nextPlayer->hand_size));
        addCardToHand(pile, c, pileSize);
        *currentColor = c.color;
    }
    
    return 1;
}

int handleNOT(int *currentPlayerIndex, int playerCount, int *direction) {
    printf("NOT card played! Next player's turn is skipped.\n");
    *currentPlayerIndex = (*currentPlayerIndex + *direction + playerCount) % playerCount;
    return 1;
}

void handleReverse(int *currentPlayerIndex, int playerCount, int *direction) {
    *direction *= -1;
    if (playerCount == 2) {
        printf("Reverse card played! Current player goes again.\n");
    } else {
        printf("Reverse card played! Turn order reversed.\n");
    }
}

void printCard(card c) {
    if (c.color == 'S') {
        switch (c.name) {
            case 'A': printf("AND"); break;
            case 'O': printf("OR"); break;
            case 'N': printf("NOT"); break;
            case 'R': printf("REVERSE"); break;
            default: printf("Special"); break;
        }
    } else {
        switch (c.color) {
            case 'R': printf("Red "); break;
            case 'Y': printf("Yellow "); break;
            case 'G': printf("Green "); break;
            case 'B': printf("Blue "); break;
            default: printf("Unknown "); break;
        }
        printf("%c", c.name);
    }
}

void printPlayerHand(player p) {
    printf("%s's hand (%d cards):\n", p.name, p.hand_size);
    for (int i = 0; i < p.hand_size; i++) {
        printf("%d: ", i);
        printCard(p.hand[i]);
        printf("\n");
    }
}

void freeHand(card *hand) {
    free(hand);
}

void addCardToHand(card **hand, card newCard, int *hand_size) {
    *hand = realloc(*hand, (*hand_size + 1) * sizeof(card));
    (*hand)[*hand_size] = newCard;
    (*hand_size)++;
}

void removeCardFromHand(card **hand, int index, int *hand_size) {
    if (*hand_size == 0 || index < 0 || index >= *hand_size) return;
    
    for (int i = index; i < *hand_size - 1; i++) {
        (*hand)[i] = (*hand)[i+1];
    }
    
    *hand = realloc(*hand, (*hand_size - 1) * sizeof(card));
    (*hand_size)--;
}

void aiPlay(player *ai, card topOfPile, card **pile, card **deck, int *deckSize, int *currentPlayerIndex, int playerCount, player *players, int *direction, char *currentColor, int *pileSize) {
    for (int i = 0; i < ai->hand_size; i++) {
        card candidate = ai->hand[i];
        if (isValidCard(topOfPile, candidate, *currentColor)) {
            printf("AI chooses to play: ");
            printCard(candidate);
            printf("\n");
            
            if (candidate.color == 'S' && (candidate.name == 'A' || candidate.name == 'O')) {
                for (int j = 0; j < ai->hand_size; j++) {
                    if (i != j && ai->hand[j].color != 'S') {
                        playCard(ai, i, pile, deck, deckSize, currentPlayerIndex, playerCount, players, direction, currentColor, pileSize);
                        playCard(ai, j, pile, deck, deckSize, currentPlayerIndex, playerCount, players, direction, currentColor, pileSize);
                        return;
                    }
                }
            } else {
                playCard(ai, i, pile, deck, deckSize, currentPlayerIndex, playerCount, players, direction, currentColor, pileSize);
                return;
            }
        }
    }
    
    if (*deckSize > 0) {
        printf("AI has no valid cards to play. Drawing a card.\n");
        drawCard(deck, deckSize, ai);
    } else {
        printf("AI has no valid cards to play and deck is empty.\n");
    }
}

void playCard(player *currentPlayer, int cardIndex, card **pile, card **deck, int *deckSize, int *currentPlayerIndex, int playerCount, player *players, int *direction, char *currentColor, int *pileSize) {
    card playedCard = currentPlayer->hand[cardIndex];
    
    removeCardFromHand(&(currentPlayer->hand), cardIndex, &(currentPlayer->hand_size));
    addCardToHand(pile, playedCard, pileSize);
    
    if (playedCard.color != 'S') {
        *currentColor = playedCard.color;
    }
    
    if (playedCard.color == 'S') {
        int nextPlayerIdx = (*currentPlayerIndex + *direction + playerCount) % playerCount;
        
        switch (playedCard.name) {
            case 'A':
                if (players != NULL) {
                    handleAND(currentPlayer, (*pile)[*pileSize-1], deck, deckSize, &players[nextPlayerIdx], currentColor, pile, pileSize);
                }
                break;
            case 'O':
                if (players != NULL) {
                    handleOR(currentPlayer, (*pile)[*pileSize-1], deck, deckSize, &players[nextPlayerIdx], currentColor, pile, pileSize);
                }
                break;
            case 'N':
                handleNOT(currentPlayerIndex, playerCount, direction);
                break;
            case 'R':
                handleReverse(currentPlayerIndex, playerCount, direction);
                break;
        }
    }
}

char getColorName(char colorCode) {
    switch (colorCode) {
        case 'R': return 'R';
        case 'Y': return 'Y';
        case 'G': return 'G';
        case 'B': return 'B';
        default: return ' ';
    }
}

int main() {
    srand(time(NULL));
    
    int playerCount;
    printf("Enter number of players (2-4): ");
    scanf("%d", &playerCount);
    getchar();
    
    if (playerCount < 2 || playerCount > 4) {
        printf("Invalid number of players. Exiting.\n");
        return 1;
    }
    
    player players[4];
    for (int i = 0; i < playerCount; i++) {
        printf("Enter player %d's name (or 'AI' for computer player): ", i+1);
        fgets(players[i].name, 50, stdin);
        players[i].name[strcspn(players[i].name, "\n")] = '\0';
        
        if (strcmp(players[i].name, "AI") == 0) {
            players[i].is_ai = true;
            snprintf(players[i].name, 50, "AI Player %d", i+1);
        } else {
            players[i].is_ai = false;
        }
        
        players[i].hand = NULL;
        players[i].hand_size = 0;
    }
    
    card *deck = NULL;
    int deckSize = 0;
    initializeDeck(&deck, &deckSize);
    shuffleDeck(&deck, &deckSize);
    
    for (int i = 0; i < 7; i++) {
        for (int j = 0; j < playerCount; j++) {
            drawCard(&deck, &deckSize, &players[j]);
        }
    }
    
    card *pile = NULL;
    int pileSize = 0;
    int currentPlayerIndex = 0;
    int direction = 1;
    bool gameOver = false;
    char currentColor = ' ';
    
    if (deckSize > 0) {
        card firstCard = deck[0];
        removeCardFromHand(&deck, 0, &deckSize);
        addCardToHand(&pile, firstCard, &pileSize);
        currentColor = (firstCard.color == 'S') ? 'R' : firstCard.color;
    }
    
    while (!gameOver) {
        player *currentPlayer = &players[currentPlayerIndex];
        card topOfPile = pile[pileSize-1];
        
        printf("\nTop of pile: ");
        printCard(topOfPile);
        printf(" (Current color: %c)\n", getColorName(currentColor));
        
        printf("%s's turn\n", currentPlayer->name);
        printPlayerHand(*currentPlayer);
        
        if (currentPlayer->is_ai) {
            aiPlay(currentPlayer, topOfPile, &pile, &deck, &deckSize, &currentPlayerIndex, playerCount, players, &direction, &currentColor, &pileSize);
        } else {
            bool validPlay = false;
            while (!validPlay) {
                printf("Enter which card to play (0-%d) or -1 to draw: ", currentPlayer->hand_size - 1);
                
                char input[10];
                fgets(input, sizeof(input), stdin);
                
                if (strcmp(input, "-1\n") == 0) {
                    if (deckSize > 0) {
                        drawCard(&deck, &deckSize, currentPlayer);
                        printf("Drew a card: ");
                        printCard(currentPlayer->hand[currentPlayer->hand_size - 1]);
                        printf("\n");
                    } else {
                        printf("Deck is empty.\n");
                    }
                    validPlay = true;
                } else {
                    int cardIndex = atoi(input);
                    if (cardIndex >= 0 && cardIndex < currentPlayer->hand_size) {
                        card candidate = currentPlayer->hand[cardIndex];
                        if (isValidCard(topOfPile, candidate, currentColor)) {
                            playCard(currentPlayer, cardIndex, &pile, &deck, &deckSize, &currentPlayerIndex, playerCount, players, &direction, &currentColor, &pileSize);
                            validPlay = true;
                            
                            if (candidate.color != 'S') {
                                currentColor = candidate.color;
                            }
                        } else {
                            printf("Invalid choice, cannot place ");
                            printCard(candidate);
                            printf(" on ");
                            printCard(topOfPile);
                            printf("\n");
                        }
                    } else {
                        printf("Invalid choice. Please try again.\n");
                    }
                }
            }
        }
        
        if (currentPlayer->hand_size == 0) {
            printf("\n%s has no cards left and wins the game!\n", currentPlayer->name);
            gameOver = true;
        } else if (deckSize == 0) {
            int minCards = players[0].hand_size;
            int winnerIndex = 0;
            for (int i = 1; i < playerCount; i++) {
                if (players[i].hand_size < minCards) {
                    minCards = players[i].hand_size;
                    winnerIndex = i;
                }
            }
            printf("\nDeck is empty. %s wins with %d cards remaining!\n", 
                   players[winnerIndex].name, players[winnerIndex].hand_size);
            gameOver = true;
        }
        
        if (!gameOver) {
            currentPlayerIndex = (currentPlayerIndex + direction + playerCount) % playerCount;
        }
    }
    
    for (int i = 0; i < playerCount; i++) {
        freeHand(players[i].hand);
    }
    freeHand(deck);
    freeHand(pile);
    
    return 0;
}
