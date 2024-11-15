#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>

#define n 4             //changing the number here would change the size of the grid (n x n)
#define p 2             //the original game is played in powers of 2. You can change that here
#define w 4             //change the width of each tile, in case higher 'p' selected

//NOTE: empty tiles are stored as 0s in the code but printed as -

//getch implementation in linux to disable echo for neater game UI
int getch() {
    system("/bin/stty raw");
    struct termios oldtc;
    struct termios newtc;
    int ch;
    tcgetattr(STDIN_FILENO, &oldtc);
    newtc = oldtc;
    newtc.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newtc);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldtc);
    system("/bin/stty cooked");
    return ch;
}

//forward declaration
void readMove(int arr[n][n], int* score, int* moves, int prev[n][n]);
int maxValue(int arr[n][n]);

int isEqual(int arr[n][n], int prev[n][n])
{
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            if (arr[i][j] != prev[i][j])
                return 0;
        }
    }
    return 1;
}

//permanent title for aesthetic reasons
void title()
{
    printf("\x1b[H\x1b[J");
    //credits for this graphic: https://github.com/plibither8/2048.cpp/blob/master/src/game-graphics.cpp"
    printf("Welcome to Dhruv Jauhar's\n\n");
    printf("   /\\\\\\\\\\\\\\\\\\          /\\\\\\\\\\\\\\                /\\\\\\         /\\\\\\\\\\\\\\\\\\\\\n");
    printf("  /\\\\\\///////\\\\\\      /\\\\\\/////\\\\\\            /\\\\\\\\\\       /\\\\\\///////\\\\\\\n");
    printf("  \\///      \\//\\\\\\    /\\\\\\    \\//\\\\\\         /\\\\\\/\\\\\\      \\/\\\\\\     \\/\\\\\\\n");
    printf("             /\\\\\\/    \\/\\\\\\     \\/\\\\\\       /\\\\\\/\\/\\\\\\      \\///\\\\\\\\\\\\\\\\\\/\n");
    printf("           /\\\\\\//      \\/\\\\\\     \\/\\\\\\     /\\\\\\/  \\/\\\\\\       /\\\\\\///////\\\\\n");
    printf("         /\\\\\\//         \\/\\\\\\     \\/\\\\\\   /\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\   /\\\\\\      \\//\\\\\n");
    printf("        /\\\\\\/            \\//\\\\\\    /\\\\\\   \\///////////\\\\\\//   \\//\\\\\\     \\/\\\\\\\n");
    printf("        /\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\   \\///\\\\\\\\\\\\\\/              \\/\\\\\\      \\///\\\\\\\\\\\\\\\\\\/\n");
    printf("        \\///////////////      \\///////                \\///         \\/////////\n\n");
}

//display board and score
void display(int arr[n][n], int score)
{
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            if (arr[i][j] != 0)
                printf("%*d ", w, arr[i][j]);
            else
            {
                for (int k = 0; k < w - 1; k++)
                    printf(" ");
                printf("- ");
            }
        }
        if (i == 0)
            printf("\t\tScore: %d", score);
        printf("\n");
    }
}

void header(int arr[n][n], int score)
{
    title();
    display(arr, score);
    printf("\nw:Up  a:Left  s:Down  d:Right  n:Next Moves  r:Reset Game  u:Undo  v:View Stats  q:Quit\n");
}
//rotate the board 90 degrees anticlockwise
void rotate(int arr[n][n])
{
    int swap;
    for (int i = 0; i < n; i++)                 //rotation is simply transpose and then reversal of columns
        for (int j = i; j < n; j++)
        {
            swap = arr[i][j];
            arr[i][j] = arr[j][i];
            arr[j][i] = swap;
        }
    for (int i = 0; i < n; i++)
        for (int j = 0, k = n - 1; j < k; j++, k--)
        {
            swap = arr[j][i];
            arr[j][i] = arr[k][i];
            arr[k][i] = swap;
        }
}

//count number of tiles of value tile
int tileCount(int arr[n][n], int tile)
{
    int count = 0;
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
            if (arr[i][j] == tile)
                count++;
    }
    return count;
}

//add new number in blank tile
void addNumber(int arr[n][n]) //randomizes the location of all possible blanks, then fills it with an integer
{
    int count = tileCount(arr, 0);
    if (count == 0)
        return;
    int pos = (int)rand() % count;        //decides a random position to input the new number out of the blank tiles

    //generate new number to be added
    //as per http://talkingdatawithsean.com/beating-2048-an-application-of-lookahead-and-tree-search-methods/
    //the new number has 90% probably of 2 and remainder as 4, or if you change the definition, p and p^2

    int new_number;
    if (rand() % 10 == 0)       //implements 10% probability
        new_number = p * p;
    else
        new_number = p;
    count = -1;
    for (int i = 0; i < n; i++)         //puts the new number in the position that was randomly selected
    {
        for (int j = 0; j < n; j++)
        {
            if (arr[i][j] == 0)
                count++;
            if (count == pos)
            {
                arr[i][j] = new_number;
                i = n;                  //simple way to break out of both loops
                j = n;
            }
        }
    }
}

//checks whether any moves possible
int gameOver(int arr[n][n])  //returns true if gameover if there are no moves left or if user quits game
{
    int count = tileCount(arr, 0);
    if (count != 0)         //blank tile exists, game not over
        return 0;
    for (int i = 0; i < n; i++) //will only enter loop if all tiles occupied
    {
        for (int j = 0; j < n - 1; j++)
        {
            if (arr[i][j] == arr[i][j + 1])   //checks if any adjacent tiles are same
                return 0;
        }
    }
    for (int j = 0; j < n; j++)
    {
        for (int i = 0; i < n - 1; i++)
        {
            if (arr[i][j] == arr[i + 1][j])   //checks if any adjacent tiles are same
                return 0;
        }
    }
    return 1;
}

//clears the board, resets score and adds 2 initial numbers to start with
void initialize(int arr[n][n], int* score)
{
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
            arr[i][j] = 0;
    }
    addNumber(arr);
    addNumber(arr);
    *score = 0;
}

//unoptimized code to push all tiles towards right
void pushRight(int arr[n][n])
{
    for (int i = 0; i < n; i++)
    {
        int num[n];
        int count = 0;
        for (int j = 0; j < n; j++)     //counts number of non blank tiles
        {
            if (arr[i][j] != 0)
            {
                num[count] = arr[i][j];
                count++;
            }
        }
        count--;
        int j;
        for (j = n - 1; count >= 0; count--, j--)   //pushes non blank tiles to the right
            arr[i][j] = num[count];
        for (;j >= 0; j--)                          //fills remainder tiles with zeroes
            arr[i][j] = 0;
    }
}

//unoptimized code to merge all tiles towards right, rest are used with mergeright and rotation
void mergeRight(int arr[n][n], int* score)
{
    pushRight(arr);     //push all tiles to the right first so theres no blanks interfering
    for (int i = 0; i < n; i++)
    {
        for (int j = n - 1; (j > 0) && (arr[i][j] != 0); j--)     //merge if consecutive tiles found
        {
            if (arr[i][j] == arr[i][j - 1])
            {
                arr[i][j] *= p;         //2 2 pushed right gets stored as 0 4, then the j-- at the end iterates over the 0
                *score += arr[i][j];
                arr[i][j - 1] = 0;
                j--;
            }
        }
    }
    pushRight(arr);     //since we created zeroes previously, push right again
}

void mergeUp(int arr[n][n], int* score)
{
    rotate(arr);
    rotate(arr);
    rotate(arr);
    mergeRight(arr, score);
    rotate(arr);
}

void mergeLeft(int arr[n][n], int* score)
{
    rotate(arr);
    rotate(arr);
    mergeRight(arr, score);
    rotate(arr);
    rotate(arr);
}

void mergeDown(int arr[n][n], int* score)
{
    rotate(arr);
    mergeRight(arr, score);
    rotate(arr);
    rotate(arr);
    rotate(arr);
}

//duplicate 2Darray arr into 2Darray temp
void assign(int temp[n][n], int arr[n][n])
{
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
            temp[i][j] = arr[i][j];
    }
}

//display next states and revert temp and score variables
void showResetTemp(int c, int arr[n][n], int temp[n][n], int* score, int orig)
{
    if (c == 'y')
        addNumber(temp);
    display(temp, *score);
    assign(temp, arr);
    *score = orig;
}

//generate and display all next states
void nextMoves(int arr[n][n], int* score)
{
    int max = 0;
    header(arr, *score);
    int c;
    printf("\nShow with addition of random tile? (y/n): ");
    c = getch();
    int temp[n][n];
    int orig = *score;
    assign(temp, arr);
    printf("\n\nUP:\n");
    mergeUp(temp, score);
    if (maxValue(temp) > max)
        max = maxValue(temp);
    showResetTemp(c, arr, temp, score, orig);
    printf("\nLEFT:\n");
    mergeLeft(temp, score);
    if (maxValue(temp) > max)
        max = maxValue(temp);
    showResetTemp(c, arr, temp, score, orig);
    printf("\nDOWN:\n");
    mergeDown(temp, score);
    if (maxValue(temp) > max)
        max = maxValue(temp);
    showResetTemp(c, arr, temp, score, orig);
    printf("\nRIGHT:\n");
    mergeRight(temp, score);
    if (maxValue(temp) > max)
        max = maxValue(temp);
    showResetTemp(c, arr, temp, score, orig);
    printf("\nHighest tile attainable in next move: %d\n", max);
}

void quitGame(int arr[n][n], int score)
{
    printf("Your total score is: %d\nYour highest tile is: %d\n", score, maxValue(arr));
    if (maxValue(arr) >= p * p * p * p * p * p * p * p * p * p * p)
        printf("YOU WIN!\n\n");
    else
        printf("YOU LOSE\n\n");
}

int maxValue(int arr[n][n])
{
    int max = 0;
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            if (arr[i][j] > max)
                max = arr[i][j];
        }
    }
    return max;
}

int sumTiles(int arr[n][n])
{
    int sum = 0;
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
            sum += arr[i][j];
    }
    return sum;
}

void stats(int arr[n][n], int* score, int* moves)
{
    int max = maxValue(arr);
    header(arr, *score);
    printf("\nHighest tile: %d\n", max);
    printf("Sum of all tiles: %d\n", sumTiles(arr));
    printf("Number of blank tiles: %d\n", tileCount(arr, 0));
    for (int i = p; i <= max; i *= p)
    {
        if (tileCount(arr, i) != 0)
            printf("Tiles of value %d: %d\n", i, tileCount(arr, i));
    }
    printf("Total moves: %d\n", *moves);
}
//read input and perform respective action
void readMove(int arr[n][n], int* score, int* moves, int prev[n][n])
{
    int c = getch();
    switch (c)
    {
    case'w':
        assign(prev, arr);
        mergeUp(arr, score);
        if (!isEqual(prev, arr))
        {
            *moves += 1;
            addNumber(arr);
        }
        break;
    case'a':
        assign(prev, arr);
        mergeLeft(arr, score);
        if (!isEqual(prev, arr))
        {
            *moves += 1;
            addNumber(arr);
        }
        break;
    case's':
        assign(prev, arr);
        mergeDown(arr, score);
        if (!isEqual(prev, arr))
        {
            *moves += 1;
            addNumber(arr);
        }
        break;
    case'd':
        assign(prev, arr);
        mergeRight(arr, score);
        if (!isEqual(prev, arr))
        {
            *moves += 1;
            addNumber(arr);
        }
        break;
    case'r':
        initialize(arr, score);
        *moves = 0;
        header(arr, *score);
        break;
    case'n':
        nextMoves(arr, score);
        readMove(arr, score, moves, prev);
        break;
    case'u':
        assign(arr, prev);
        *moves-=1;
        break;
    case'v':
        stats(arr, score, moves);
        readMove(arr, score, moves, prev);
        break;
    case'q':
        title();
        display(arr, *score);
        printf("\nQUIT!\n");
        quitGame(arr, *score);
        exit(0);
        break;
    default:
        break;
    }
}

void user(int arr[n][n], int prev[n][n])
{
    int score;
    int moves = 0;
    initialize(arr, &score);
    assign(prev, arr);
    while (!gameOver(arr))
    {
        header(arr, score);
        readMove(arr, &score, &moves, prev);
    }
    title();
    display(arr, score);
    printf("\nGAME OVER!\n");
    quitGame(arr, score);
}

int main()
{
    title();
    printf("Press u to start user-played game, any other key to start driver code\n\n");
    int c;
    c = getch();
    srand((unsigned int)time(0));       //for proper randomization
    int arr[n][n];
    int prev[n][n];
    if (c == 'u')
    {
        user(arr, prev);
        exit(0);
    }
    int score;
    initialize(arr, &score);
    int count = 0;
    display(arr, score);
    printf("\nSum of tiles is %d", sumTiles(arr));
    while (sumTiles(arr) != 8)
    {
        switch (count % 4)
        {
        case 0:
            assign(prev, arr);
            mergeUp(arr, &score);
            if (!isEqual(prev, arr))
            {
                addNumber(arr);
                printf("\nMove Up\n\n");
                display(arr, score);
            }
            count++;
            break;
        case 1:
            assign(prev, arr);
            mergeRight(arr, &score);
            if (!isEqual(prev, arr))
            {
                addNumber(arr);
                printf("\nMove Right\n\n");
                display(arr, score);
            }
            count++;
            break;
        case 2:
            assign(prev, arr);
            mergeDown(arr, &score);
            if (!isEqual(prev, arr))
            {
                addNumber(arr);
                printf("\nMove Down\n\n");
                display(arr, score);
            }
            count++;
            break;
        case 3:
            assign(prev, arr);
            mergeLeft(arr, &score);
            if (!isEqual(prev, arr))
            {
                addNumber(arr);
                printf("\nMove Left\n\n");
                display(arr, score);
            }
            count++;
            break;
        }
        printf("\nSum of tiles is %d", sumTiles(arr));
        if (sumTiles(arr) > 8)
        {
            assign(arr, prev);
            printf("\nUndo\n\n");
            display(arr, score);
        }
    }
    printf("\nGame Over! Sum of tiles is now 8\n");
    return 0;
}
