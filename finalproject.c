// James Washburn and Nico's Final Code! //
// Our Uno Project:  //
// It doesn't output as the exact same as the example //
// on D2L but went to Office Hours and Mai said that //
// It was just fine because it still worked and  //
// had all the important info still involved and applied. //
// Honestly, super fun project but very hard and time consuming - feels good to be finished. //

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <ctype.h>

// Definied a card node in a linked list which we were having problems with but also this holds its name, color, and pointer to the next card.
typedef struct card_s {
    char name; // card value so either numerical 1,2,3... or 'A' for AND...
    char color; 
    struct card_s *next; //points to the next card in list
} card;

// Defined a player, with a name, dynamic hand array, hand size, and AI flag
typedef struct {
    char name[50];//player name
    card *hand;//referencing our dynamic array of cards, specifically their hand
    int hand_size; //amouunt of cards ina  players hand
    bool is_ai; //Only True if one of the players is selected to be "AI"
} player;

// All of the many required function prototypes for the game
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


// Builds the base UNO deck of number cards + special cards
void initializeDeck(card **deck, int *deckSize) {
    char colors[] = {'R', 'Y', 'G', 'B'}; //red yello green blue
    char numbers[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
    char specials[] = {'A', 'O', 'N', 'R'}; //special cards abbreiviated

// Add number cards with two copies of each number for each color
    for (int c = 0; c < 4; c++) { //for 4 different colors
        for (int n = 0; n < 10; n++) { //1-9
            for (int i = 0; i < 2; i++) {
                card newCard = {numbers[n], colors[c], NULL}; //creates card instance
                addCardToHand(deck, newCard, deckSize); //calls function to actually add card to deck
            }
        }
    }
 // Add special cards to the deck's, five of each special type, color code 'S' as special. 
    for (int s = 0; s < 4; s++) { 
        for (int i = 0; i < 5; i++) { //same logic as above but for special
            card newCard = {specials[s], 'S', NULL};
            addCardToHand(deck, newCard, deckSize);
        }
    }
}
// Randomly swaps pairs of cards many times to shuffle deck - In D2L Rubric it said to do 10,000 which we did but sometimes crashes.
void shuffleDeck(card **deck, int *deckSize) {
    for (int i = 0; i < 10000; i++) { //shuffles deck 10000 times
        int index1 = rand() % *deckSize;
        int index2 = rand() % *deckSize; //using required rand() command 
        
        card temp = (*deck)[index1]; //logic here is it works by swapping pairs of cards(this will iterate 10000 times)
        (*deck)[index1] = (*deck)[index2];
        (*deck)[index2] = temp;
    }
}
// Remove top card of deck and add it to player's hand if they draw a card
int drawCard(card **deck, int *deckSize, player *p) {
    if (*deckSize == 0) return 0;
    
    card drawnCard = (*deck)[0]; //pulls card first card(index 0)
    removeCardFromHand(deck, 0, deckSize);
    addCardToHand(&(p->hand), drawnCard, &(p->hand_size));
    return 1;
}
// Validates if a card can be played on the current top card
int isValidCard(card topOfPile, card candidate, char currentColor) {
// Special cards can always be played except AND/OR which have additional requirements and cant be played on top of another special card.
    if (candidate.color == 'S') {
// AND/OR can only be played on normal number cards (0-9)
        if (candidate.name == 'A' || candidate.name == 'O') {
            return (topOfPile.color != 'S'); // Only valid if top card is not special
        }
        return 1; // Other special cards (NOT, REVERSE) can be played anytime
    }
    
// Normal cards must match color or number to be played
    return (candidate.color == currentColor) || (candidate.name == topOfPile.name);
}
//Logic for AND card - next card after AND must match both color and number or other player must draw 4.
int handleAND(player *currentPlayer, card topOfPile, card **deck, int *deckSize, player *nextPlayer, char *currentColor, card **pile, int *pileSize) {
// Get the actual paired number card from the pile (In our function we have it as second to last played card)
    char pairedNumber = (*pile)[*pileSize-2].name;
    char pairedColor = (*pile)[*pileSize-2].color;

    printf("%s played AND with ", currentPlayer->name);
    printCard((*pile)[*pileSize-2]); //prints paired card(second to last card)
    printf(". %s must play a card matching BOTH color (%c) and number (%c) or draw 4.\n", 
           nextPlayer->name, getColorName(pairedColor), pairedNumber);

// The bool below checks the next player's hand for exact match
    bool hasMatch = false;

// search through next player's hand for exact match
for (int i = 0; i < nextPlayer->hand_size; i++) {
    // get current card from hand
    card c = nextPlayer->hand[i];
    
    // check if card matches both color and number
    if (c.color == pairedColor && c.name == pairedNumber) {
        hasMatch = true;  // set flag if match found
        break;            // exit loop early if found
    }
}

// if no matching card was found
if (!hasMatch) {
    // tell player they must draw
    printf("%s doesn't have a matching card. Drawing 4 cards.\n", nextPlayer->name);
    
    // make them draw 4 cards
    for (int i = 0; i < 4; i++) {
        // attempt to draw card
        if (!drawCard(deck, deckSize, nextPlayer)) {
            // if deck is empty, stop drawing
            printf("Deck is empty!\n");
            break;
        }
    }
    return 0;  // return 0 to indicate penalty was applied
}

// if we get here, player has matching card - find and play it
for (int i = 0; i < nextPlayer->hand_size; i++) {
    // get current card from hand
    card c = nextPlayer->hand[i];
    
    // check for matching card
    if (c.color == pairedColor && c.name == pairedNumber) {
        // announce the played card
        printf("%s plays matching card: ", nextPlayer->name);
        printCard(c);
        printf("\n");
        
        // remove card from player's hand
        removeCardFromHand(&(nextPlayer->hand), i, &(nextPlayer->hand_size));
        
        // add card to pile
        addCardToHand(pile, c, pileSize);
        
        // update current color
        *currentColor = c.color;
        
        return 1;  // return 1 to indicate successful play
    }
}

// default return (shouldn't reach here if logic is correct)
return 1;
}

// get the card that was played before the OR (second-to-last in pile)
char pairedNumber = (*pile)[*pileSize-2].name;
char pairedColor = (*pile)[*pileSize-2].color;

// print OR card play message
printf("%s played OR with ", currentPlayer->name);
printCard((*pile)[*pileSize-2]);

// explain OR card rules
printf(". %s must play a card matching EITHER color (%c) OR number (%c) or draw 4.\n",
       nextPlayer->name, getColorName(pairedColor), pairedNumber);

// initialize match tracking variables
bool hasMatch = false;
int matchIndex = -1;

// search hand for any card matching either color OR number
for (int i = 0; i < nextPlayer->hand_size; i++) {
    card c = nextPlayer->hand[i];
    if (c.color == pairedColor || c.name == pairedNumber) {
        hasMatch = true;  // mark match found
        matchIndex = i;   // remember position
        break;            // stop at first match
    }
}

// if no matching card found
if (!hasMatch) {
    printf("%s doesn't have a matching card. Drawing 4 cards.\n", nextPlayer->name);
    
    // draw 4 penalty cards
    for (int i = 0; i < 4; i++) {
        if (!drawCard(deck, deckSize, nextPlayer)) {
            printf("Deck is empty!\n");
            break;
        }
    }
    return 0;  // return 0 for penalty applied
}

// if match was found
if (matchIndex != -1) {
    card c = nextPlayer->hand[matchIndex];
    
    // announce the play
    printf("%s plays matching card: ", nextPlayer->name);
    printCard(c);
    printf("\n");
    
    // move card from hand to pile
    removeCardFromHand(&(nextPlayer->hand), matchIndex, &(nextPlayer->hand_size));
    addCardToHand(pile, c, pileSize);
    
    // update current color
    *currentColor = c.color;
}

return 1; 
}

int handleNOT(int *currentPlayerIndex, int playerCount, int *direction) {
    printf("NOT card played! Next player's turn is skipped.\n");
    *currentPlayerIndex = (*currentPlayerIndex + *direction + playerCount) % playerCount; //finds direction and sets the player index ahead skippig current player
    return 1;
}

void handleReverse(int *currentPlayerIndex, int playerCount, int *direction) {
    *direction *= -1; // reverse the turn direction
    if (playerCount == 2) {
        printf("Reverse card played! Current player goes again.\n"); //specifically for if its a two player game
    } else {
        printf("Reverse card played! Turn order reversed.\n");
    }
}


// Prints a card to console and says if its "S" which is special or the color.
void printCard(card c) {
    // check if card is special (color 'S')
if (c.color == 'S') {
    // print special card name
    switch (c.name) {
        case 'A': printf("AND"); break;
        case 'O': printf("OR"); break;
        case 'N': printf("NOT"); break;
        case 'R': printf("REVERSE"); break;
        default: printf("Special"); break;
    }
} else {
    // print color prefix for normal cards
    switch (c.color) {
        case 'R': printf("Red "); break;
        case 'Y': printf("Yellow "); break;
        case 'G': printf("Green "); break;
        case 'B': printf("Blue "); break;
        default: printf("Unknown "); break;
    }
    // print card value/number
    printf("%c", c.name);
}
}
// The code above Prints the color prefix
// Code below displays the players current hand 
void printPlayerHand(player p) {
    printf("%s's hand (%d cards):\n", p.name, p.hand_size); //playername and plalery hand ssize
    for (int i = 0; i < p.hand_size; i++) {
        printf("%d: ", i);
        printCard(p.hand[i]);
        printf("\n");
    }
}
// Frees a dynamically allocated hand array
void freeHand(card *hand) {
    free(hand);
}

void addCardToHand(card **hand, card newCard, int *hand_size) {
    // resizes hand array for new card
*hand = realloc(*hand, (*hand_size + 1) * sizeof(card));

// adds new card at the end
(*hand)[*hand_size] = newCard;

// increments hand size counter
(*hand_size)++;
}

// Adds a new card to the dynamic hand or deck array, resizing with the realloc function
void removeCardFromHand(card **hand, int index, int *hand_size) {
   // safety check for invalid index
if (*hand_size == 0 || index < 0 || index >= *hand_size) return;

// shifst all cards after index left by one position
for (int i = index; i < *hand_size - 1; i++) {
    (*hand)[i] = (*hand)[i+1];
}

// resizes array to new smaller size
*hand = realloc(*hand, (*hand_size - 1) * sizeof(card));

// decrements tghe hand size counter
(*hand_size)--;
}

// handles AI player's turn logic
void aiPlay(player *ai, card topOfPile, card **pile, card **deck, int *deckSize, int *currentPlayerIndex, int playerCount, player *players, int *direction, char *currentColor, int *pileSize) {
    // scan through AI's hand for playable cards
    for (int i = 0; i < ai->hand_size; i++) {
        card candidate = ai->hand[i];
        
        // check if card is valid to play
        if (isValidCard(topOfPile, candidate, *currentColor)) {
            // announce AI's choice
            printf("AI chooses to play: ");
            printCard(candidate);
            printf("\n");
            
            // special handling for AND/OR cards (need a second card)
            if (candidate.color == 'S' && (candidate.name == 'A' || candidate.name == 'O')) {
                // search for a second non-special card to pair
                for (int j = 0; j < ai->hand_size; j++) {
                    if (i != j && ai->hand[j].color != 'S') {
                        // play both cards
                        playCard(ai, i, pile, deck, deckSize, currentPlayerIndex, playerCount, players, direction, currentColor, pileSize);
                        playCard(ai, j, pile, deck, deckSize, currentPlayerIndex, playerCount, players, direction, currentColor, pileSize);
                        return;
                    }
                }
            } else {
                // play single valid card
                playCard(ai, i, pile, deck, deckSize, currentPlayerIndex, playerCount, players, direction, currentColor, pileSize);
                return;
            }
        }
    }
    
    // if no valid cards found, draw a card
    if (*deckSize > 0) {
        printf("AI has no valid cards to play. Drawing a card.\n");
        drawCard(deck, deckSize, ai);
    } else {
        printf("AI has no valid cards to play and deck is empty.\n");
    }
}

// executes playing a card and handles all effects
void playCard(player *currentPlayer, int cardIndex, card **pile, card **deck, int *deckSize, int *currentPlayerIndex, int playerCount, player *players, int *direction, char *currentColor, int *pileSize) {
    // get the card being played
    card playedCard = currentPlayer->hand[cardIndex];
    
    // remove card from player's hand
    removeCardFromHand(&(currentPlayer->hand), cardIndex, &(currentPlayer->hand_size));
    
    // add card to discard pile
    addCardToHand(pile, playedCard, pileSize);
    
    // update current color if not a special card
    if (playedCard.color != 'S') {
        *currentColor = playedCard.color;
    }
    
    // handle special card effects
    if (playedCard.color == 'S') {
        // calculate next player's index
        int nextPlayerIdx = (*currentPlayerIndex + *direction + playerCount) % playerCount;
        
        // check which special card was played
        switch (playedCard.name) {
            case 'A':  // AND card
                if (players != NULL) {
                    handleAND(currentPlayer, (*pile)[*pileSize-1], deck, deckSize, &players[nextPlayerIdx], currentColor, pile, pileSize);
                }
                break;
            case 'O':  // OR card
                if (players != NULL) {
                    handleOR(currentPlayer, (*pile)[*pileSize-1], deck, deckSize, &players[nextPlayerIdx], currentColor, pile, pileSize);
                }
                break;
            case 'N':  // NOT card
                handleNOT(currentPlayerIndex, playerCount, direction);
                break;
            case 'R':  // REVERSE card
                handleReverse(currentPlayerIndex, playerCount, direction);
                break;
        }
    }
}

// Then it returns the character representing a color code
char getColorName(char colorCode) {
    switch (colorCode) {
        case 'R': return 'R';//for red
        case 'Y': return 'Y'; //yelllow
        case 'G': return 'G'; //green
        case 'B': return 'B'; //blue
        default: return ' '; //for special cards that dont have a color
    }
}
// Now the fun begins, in the main function we setup the game with players, the deck, and it runs the main game loop
int main() {
    // seed random number generator for shuffling
    srand(time(NULL)); //using required function in main(srand())
    
    // get number of players
    int playerCount;
    printf("Enter number of players (2+): ");
    scanf("%d", &playerCount);
    getchar();  // consume newline
    
    // validate player count
    if (playerCount < 2) {
        printf("Invalid number of players. Exiting.\n");
        return 1;
    }
    
    // allocate memory for players
    player *players = malloc(playerCount * sizeof(player)); ///using MALLOC (required)
    
    // initialize each player
    for (int i = 0; i < playerCount; i++) {
        // get player name
        printf("Enter player %d's name (or 'AI' for computer player): ", i+1);
        fgets(players[i].name, 50, stdin);
        players[i].name[strcspn(players[i].name, "\n")] = '\0';  // remove newline to make sure we get the best input
        
        // set AI flag and format name
        if (strcmp(players[i].name, "AI") == 0) {
            players[i].is_ai = true;
            snprintf(players[i].name, 50, "AI Player %d", i+1);
        } else {
            players[i].is_ai = false;
        }
        
        // initialize empty hand
        players[i].hand = NULL;
        players[i].hand_size = 0;
    }
    
    // initialize and shuffle deck
    card *deck = NULL;
    int deckSize = 0;
    initializeDeck(&deck, &deckSize); //calling initalize funciton
    shuffleDeck(&deck, &deckSize); //calling shuffle function
    
    // deal 7 cards to each player
    for (int i = 0; i < 7; i++) {
        for (int j = 0; j < playerCount; j++) {
            drawCard(&deck, &deckSize, &players[j]);
        }
    }
    
    // sets up the game state
    card *pile = NULL;  //this is the discard pile
    int pileSize = 0;
    int currentPlayerIndex = 0;
    int direction = 1;  // 1 = clockwise, -1 = counter-clockwise
    bool gameOver = false;
    char currentColor = ' ';
    
    // draw first card to start pile
    if (deckSize > 0) {
        card firstCard = deck[0];
        removeCardFromHand(&deck, 0, &deckSize);
        addCardToHand(&pile, firstCard, &pileSize);
        // default to red if first card is special
        currentColor = (firstCard.color == 'S') ? 'R' : firstCard.color;
    }
    
    // main game loop
    while (!gameOver) {
        player *currentPlayer = &players[currentPlayerIndex];
        card topOfPile = pile[pileSize-1];
        
        // display game state
        printf("\nTop of pile: ");
        printCard(topOfPile);
        printf(" (Current color: %c)\n", getColorName(currentColor));
        
        printf("%s's turn\n", currentPlayer->name);
        printPlayerHand(*currentPlayer);
        
        // handles AI player turn
        if (currentPlayer->is_ai) {
            aiPlay(currentPlayer, topOfPile, &pile, &deck, &deckSize, &currentPlayerIndex, playerCount, players, &direction, &currentColor, &pileSize);
        } 
        // handles human turn
        else {
            bool validPlay = false;
            while (!validPlay) {
                printf("Enter which card to play (0-%d) or -1 to draw: ", currentPlayer->hand_size - 1);
                
                char input[10];
                fgets(input, sizeof(input), stdin);
                
                // handles the -1 draw a card option
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
                    // handles the card play
                    int cardIndex = atoi(input);
                    if (cardIndex >= 0 && cardIndex < currentPlayer->hand_size) {
                        card candidate = currentPlayer->hand[cardIndex];
                        if (isValidCard(topOfPile, candidate, currentColor)) {
                            playCard(currentPlayer, cardIndex, &pile, &deck, &deckSize, &currentPlayerIndex, playerCount, players, &direction, &currentColor, &pileSize);
                            validPlay = true;
                            
                            // update color if not special
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
        
        // checks for winner (when the hand is empty)
        if (currentPlayer->hand_size == 0) {
            printf("\n%s has no cards left and wins the game!\n", currentPlayer->name);
            gameOver = true;
        } 
        // if deck is empty, player with fewest cards wins
        else if (deckSize == 0) {
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
            gameOver = true; //game end
        }
        
        // move to next player if game isn't over yet
        if (!gameOver) {
            currentPlayerIndex = (currentPlayerIndex + direction + playerCount) % playerCount;
        }
    }
    
    // cleanup - free all allocated memory
    for (int i = 0; i < playerCount; i++) {
        freeHand(players[i].hand);
    }
//frees memory
    freeHand(deck);
    freeHand(pile);
    free(players);
    
    return 0;
}
