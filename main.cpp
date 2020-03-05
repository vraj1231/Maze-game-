/*
 * MAZE Game Framework
 * Written by Dr. Dhanyu Amarasinghe Spring 2018
 */

#include <string.h>
#include <CommonThings.h>
#include <Maze.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <Timer.h>
#include <player.h>
#include <vector>
#include <utility>
#include <unordered_map>
#include <stack>
#include <list>

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <stdlib.h>
#include <Enemies.h>

#include <wall.h>
#include <math.h>
#include <graph.h>

/* GLUT callback Handlers */

using namespace std;

const int mazeSize = 10;
Maze *M = new Maze(mazeSize);                   // Set maze grid size
Maze *menu = new Maze(mazeSize);                // Used for displaying menus screen
Maze *won = new Maze(mazeSize);                 // Used for displaying "Player has won"  screen
Maze *lost = new Maze(mazeSize);                // Used for displaying "Player has lost" screen
int **myMatrix;                                 // Matrix used for program

bool game = false;                              // Used to change from menu to game screen

const int wallAmount = mazeSize * mazeSize;     // Maximum number of walls
int wallCounter = 1;                            // Counter for walls
wall W[wallAmount];                             // Wall with number of tile

const int verticesCount = wallAmount;           // Amount of maximum vertices
graph g(verticesCount);                         // Graph object used for AI

const int enemyAmount = 10;                      // Amount of enemies
int enemyCounter = 0;                           // Counter for amount of enemies placed into matrix
Enemies E[10];                                  // create number of enemies

Player *P = new Player();                       // create player
Timer *T0 = new Timer();                        // animation timer

float wWidth, wHeight;                          // display window width and Height
float xPos,yPos;                                // Viewpoar mapping

void display(void);                             // Main Display : this runs in a loop

void resize(int width, int height)              // resizing case on the window
{
    wWidth = width;
    wHeight = height;

    if(width<=height)
        glViewport(0,(GLsizei) (height-width)/2,(GLsizei) width,(GLsizei) width);
    else
        glViewport((GLsizei) (width-height)/2 ,0 ,(GLsizei) height,(GLsizei) height);
}

//Places wall into matrix with coordinates passed into function
void placeWall(int a, int b){
    if(wallCounter < wallAmount){
        W[wallCounter].wallInit(M->getGridSize(),"images/wall.png");// Load walls
        W[wallCounter].placeWall(a, b);
        wallCounter++;
    }
    else {
        cout << "Too many walls defined in text file!\n";
        return;
    }
}

//Places enemy into matrix with coordinates passed into function
void placeEnemy(int a, int b){
    if(enemyCounter < enemyAmount){
        E[enemyCounter].initEnm(M->getGridSize(),4,"images/chicken.png"); //Load enemy image
        E[enemyCounter].placeEnemy(a,b);
        enemyCounter++;
    }
    else{
        cout << "Too many enemies defined in text file!\n";
        return;
    }
}

// Reads level text file and places objects(walls, player, enemies...) into maze using values 1-5:
//  1 - Wall
//  2 - Enemy
//  3 - Player
//  4 - Chest
//  5 - Set of arrows
void readFile(string textFile)
{
    int a, b;
    string line, label;
    ifstream infile(textFile);

    if(infile.is_open()){
        while(getline(infile, line)){
            istringstream ss(line);
            ss >> label >> a >> b;

            if(label == "wall"){
                cout << "Wall found at: " << a << "," << b << endl;
                placeWall(a, b);
                myMatrix[a][b] = 1;
            }
            if(label == "enemy"){
                cout << "Enemy placed at: " << a << "," << b << endl;
                placeEnemy(a, b);
                myMatrix[a][b] = 2;
            }
            if(label == "player"){
                cout << "player placed at: " << a << "," << b << endl;
                P->initPlayer(M->getGridSize(),4,"images/llama.png");   // initialize player pass grid size,image and number of frames
                P->loadArrowImage("images/arr.png");                // Load arrow image
                P->placePlayer(a, b);
                myMatrix[a][b] = 3;
            }
            if(label == "arrow"){
                cout << "Arrows placed at: " << a << "," << b << endl;
                M->loadSetOfArrowsImage("images/arrwset.png");      // load set of arrows image
                M->placeStArrws(a, b);
                myMatrix[a][b] = 5;
            }
            if(label == "chest"){
                cout << "Chest found at: " << a << "," << b << endl;
                M->loadBackgroundImage("images/bak.jpg");           // Load maze background image
                M->loadChestImage("images/chest.png");              // load chest image
                M->placeChest(a, b);
                myMatrix[a][b] = 4;
            }
        }
    }
}

// Initializes matrix with 0's
void initMatrix(){
	myMatrix = new int*[mazeSize];
	for (int i = 0; i < mazeSize; i++) {
		myMatrix[i] = new int[mazeSize];
		for (int j = 0; j < mazeSize; j++)
			myMatrix[i][j] = 0;
	}
}

// Prints matrix with current values
void PrintMatrix() {
	for (int i = mazeSize - 1; i >= 0; i--) {
		for (int j = 0; j < mazeSize; j++)
			cout << myMatrix[j][i] << " ";
		cout << endl;
	}
	cout << endl;
}

// Deletes all objects created that used 'new'
void deleteAll(){
	for (int i = 0; i < mazeSize; i++)
		delete [] myMatrix;
	delete[] myMatrix;
	delete P;
	delete T0;
	delete M;
}

void init()
{
    glEnable(GL_COLOR_MATERIAL);

    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POLYGON_SMOOTH);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glClearColor(0.0,0.0,0.0,0.0);
    gluOrtho2D(0, wWidth, 0, wHeight);

    T0->Start();                                        // set timer to 0

    glEnable(GL_BLEND);                                 //display images with transparent
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    menu->loadBackgroundImage("images/flippedmenu.png");
    won->loadBackgroundImage("images/won.png");
    lost->loadBackgroundImage("images/gameOver.png");

    initMatrix();                                       // Initializes the matrix
    PrintMatrix();                                      // Prints current values from matrix
    cout << endl;

}

void display(void)
{
    glClear (GL_COLOR_BUFFER_BIT);        //ADDED THESE FUNCTOIONS FOR MENU BACKGROUND

    glPushMatrix();
    menu->drawBackground();
    glPopMatrix();

    if(game){                           //ADDED IF STATEMENT FOR THE MENU SCREEN, IF TRUE DISPLAY THE MAZE

        glPushMatrix();
        M->drawBackground();            //Displays the maze background
        glPopMatrix();

        if(M->gameOver == true){        //IF CHEST IF AQCUIRED DISPLAY THE WIN SCREEN
            static const auto hasWon = [] { PrintMatrix(); return true;}();

            //glClear (GL_COLOR_BUFFER_BIT);        // clear display screen
            glPushMatrix();
            won->drawBackground();          //USE THIS TO DISPLAY WIN SCREEN
            glPopMatrix();
           // glutSwapBuffers();


        }
        else if(lost->gameOver == true){    //ADDED THIS STATEMENT FOR DEATH SCREEN
            glPushMatrix();
            lost->drawBackground();
            glPopMatrix();
        }
        else {
            glClear (GL_COLOR_BUFFER_BIT);

            glPushMatrix();
             M->drawBackground();
            glPopMatrix();


            for(int i=0; i <= wallAmount;i++)
            {
               W[i].drawWall();
            }

            glPushMatrix();
                M->drawGrid();
            glPopMatrix();

            glPushMatrix();
                P->drawplayer();
            glPopMatrix();

            for(int i=0; i < enemyAmount ;i++)
            {
            E[i].drawEnemy();
            }

            glPushMatrix();
                P->drawArrow();
            glPopMatrix();

             glPushMatrix();
               M->drawChest();
            glPopMatrix();

            glPushMatrix();
               M->drawArrows();
            glPopMatrix();


        //glutSwapBuffers();
        }
    }
    glutSwapBuffers();          //ADDED THIS AT THE END
}


 void GetOGLPos(int x, int y)
{
    GLint viewport[4];
    GLdouble modelview[16];
    GLdouble projection[16];
    GLfloat winX, winY, winZ;
    GLdouble posX, posY, posZ;

    glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
    glGetDoublev( GL_PROJECTION_MATRIX, projection );
    glGetIntegerv( GL_VIEWPORT, viewport );

    winX = (float)x;
    winY = (float)viewport[3] - (float)y;
    glReadPixels( x, int(winY), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ );

    gluUnProject( winX, winY, winZ, modelview, projection, viewport, &posX, &posY, &posZ);

    xPos =posX ;
    yPos =posY ;
}

//Function that loops as game is running
 void idle(void)
{
    // Ends game when player collides with enemy and outputs message that player has lost
    for(int i  = 0; i < enemyCounter; i++){
        if(E[i].getEnemyLoc().x == P->getPlayerLoc().x && E[i].getEnemyLoc().y == P->getPlayerLoc().y){
            static const auto hasDied = [] { cout << "Player has died!" << endl; return true;}();
            P->livePlayer = false;
            lost->gameOver = 1;                                        //
            myMatrix[P->getPlayerLoc().x][P->getPlayerLoc().y] = 2;
        }
    }

    //If arrow is flying (has been shot)
    if(P->arrowStatus == 1){
        if(P->getArrowLoc().x - 1 >= -1 && P->getArrowLoc().x + 1 <= 10){
            //Collision with arrow and walls
            if(myMatrix[P->getArrowLoc().x][P->getArrowLoc().y] == 1)
                P->arrowStatus = 0;

            //Collisions with arrows and enemies
            for(int i = 0; i < enemyCounter; i++){
                if(E[i].getEnemyLoc().x == P->getArrowLoc().x && E[i].getEnemyLoc().y == P->getArrowLoc().y){
                    E[i].live = false;
                    myMatrix[E[i].getEnemyLoc().x][E[i].getEnemyLoc().y] = 0;
                    E[i].placeEnemy(mazeSize + 1, mazeSize + 1);
                    P->arrowStatus = false;
                }
            }
        }
    }

    glutPostRedisplay();
}

void mouse(int btn, int state, int x, int y){

    switch(btn){
        case GLUT_LEFT_BUTTON:

        if(state==GLUT_DOWN){

              GetOGLPos(x,y);
            }
            break;


      case GLUT_RIGHT_BUTTON:

        if(state==GLUT_DOWN){

              GetOGLPos(x,y);
            }
            break;
    }
     glutPostRedisplay();
};

// Allows player to shoot when player reaches set of arrows
void checkArrows(int x, int y){
    if(myMatrix[x][y] == 5){
        cout << "Player has gathered arrows!\n";
        P->hasArrows = true;
        M->liveSetOfArrws = false;
    }
}

// Ends the game when player reaches the chest
void checkChest(int x, int y){
    if(myMatrix[x][y] == 4){
        cout << "Player has won!\n";
        M->liveChest = 0;
        M->gameOver = 1; //Going to be used for displaying new screen
    }
}

// Moves enemies based on depth-first search
//  - source is enemy's current location
//  - goal is player's position
// Moves based on actions returned from dfs
void moveEnemies(){
    for(int i = 0; i < enemyCounter; i ++){
        if(E[i].live){
            //cout << "Enemy " << i << " path: ";
            g.DFS(g.returnKey(E[i].getEnemyLoc().x, E[i].getEnemyLoc().y), P);
            //cout << "\nEnemy [" << i << "] - " << "g.returnAction() returns: (" << g.returnAction().first << ", " << g.returnAction().second << ")\n";
            if(E[i].getEnemyLoc().x - g.returnAction().first == -1 && E[i].getEnemyLoc().y - g.returnAction().second == 0){ //If action returned is right
                myMatrix[E[i].getEnemyLoc().x][E[i].getEnemyLoc().y] = 0;       //Clears matrix with enemy's previous location
                myMatrix[E[i].getEnemyLoc().x + 1][E[i].getEnemyLoc().y] = 2;   //Updates matrix with enemy's new location
                E[i].moveEnemy("right");                                       //Moves enemy[i] right one spot
                g.clearVector();                                                //Clears vector containing moves
            }
            else if(E[i].getEnemyLoc().x - g.returnAction().first == 1 && E[i].getEnemyLoc().y - g.returnAction().second == 0){ //If action returned is left
                myMatrix[E[i].getEnemyLoc().x][E[i].getEnemyLoc().y] = 0;
                myMatrix[E[i].getEnemyLoc().x - 1][E[i].getEnemyLoc().y] = 2;
                E[i].moveEnemy("left");
                g.clearVector();
            }
            else if(E[i].getEnemyLoc().x - g.returnAction().first == 0 && E[i].getEnemyLoc().y - g.returnAction().second == 1){ //If action returned is down
                myMatrix[E[i].getEnemyLoc().x][E[i].getEnemyLoc().y] = 0;
                myMatrix[E[i].getEnemyLoc().x][E[i].getEnemyLoc().y - 1] = 2;
                E[i].moveEnemy("down");
                g.clearVector();
            }
            else if(E[i].getEnemyLoc().x - g.returnAction().first == 0 && E[i].getEnemyLoc().y - g.returnAction().second == -1){ //If action returned is up
                myMatrix[E[i].getEnemyLoc().x][E[i].getEnemyLoc().y] = 0;
                myMatrix[E[i].getEnemyLoc().x][E[i].getEnemyLoc().y + 1] = 2;
                E[i].moveEnemy("up");
                g.clearVector();
            }
            //cout << endl;
        }
    }
}

// Function contains player movement
// - If next step is not a wall - > move to next step
//   - If next step is the same spot as the set of arrows, allow player to shoot
//   - If next step is the saem spot as the chest, player wins
//   - Moves player into next step (from keyboard input)
//   - Updates matrix based on player movement
//   - Moves enemies after player has moved
void Specialkeys(int key, int x, int y)
{
    switch(key)
    {
    case GLUT_KEY_UP:
         if(P->shootMode == false){ //Walking mode
             if(P->getPlayerLoc().y + 1 < mazeSize){
                 if(!(myMatrix[P->getPlayerLoc().x][P->getPlayerLoc().y + 1] == 1)){
                    checkArrows(P->getPlayerLoc().x, P->getPlayerLoc().y + 1);       //Checks if next step contains is the set of arrows
                    checkChest(P->getPlayerLoc().x, P->getPlayerLoc().y + 1);        //Checks if next step

                    myMatrix[P->getPlayerLoc().x][P->getPlayerLoc().y] = 0;
                    P->movePlayer("up");                                             //Moves player up one tile
                    myMatrix[P->getPlayerLoc().x][P->getPlayerLoc().y] = 3;

                    moveEnemies();                                                   //Calls function to move enemies on player movement

                    cout << endl;
                    PrintMatrix();
                 }
            }
         }
         else if(P->hasArrows == true){ //Shooting mode
            P->playerDir = "up";
            P->shootArrow();
         }
         break;


    case GLUT_KEY_DOWN:
         if(P->shootMode == false){ //Walking mode
             if(P->getPlayerLoc().y - 1 >= 0){
                 if(!(myMatrix[P->getPlayerLoc().x][P->getPlayerLoc().y - 1] == 1)){
                    checkArrows(P->getPlayerLoc().x, P->getPlayerLoc().y - 1);
                    checkChest(P->getPlayerLoc().x, P->getPlayerLoc().y - 1);

                    myMatrix[P->getPlayerLoc().x][P->getPlayerLoc().y] = 0;
                    P->movePlayer("down");                                          //Moves player down one tile
                    myMatrix[P->getPlayerLoc().x][P->getPlayerLoc().y] = 3;

                    moveEnemies();

                    cout << endl;
                    PrintMatrix();
                 }
             }
         }
         else if(P->hasArrows == true){ //Shooting mode
            P->playerDir = "down";
            P->shootArrow();
         }
         break;

    case GLUT_KEY_LEFT:
        if(P->shootMode == false){ //Walking mode
            if(P->getPlayerLoc().x -1 >= 0){
                if(!(myMatrix[P->getPlayerLoc().x - 1][P->getPlayerLoc().y] == 1)){
                    checkArrows(P->getPlayerLoc().x - 1, P->getPlayerLoc().y);
                    checkChest(P->getPlayerLoc().x - 1, P->getPlayerLoc().y);

                    myMatrix[P->getPlayerLoc().x][P->getPlayerLoc().y] = 0;
                    P->movePlayer("left");                                          //Moves player left one tile
                    myMatrix[P->getPlayerLoc().x][P->getPlayerLoc().y] = 3;

                    moveEnemies();

                    cout << endl;
                    PrintMatrix();
                }
            }
        }
        else if(P->hasArrows == true){ //Shooting mode
            P->playerDir = "left";
            P->shootArrow();
        }
         break;

    case GLUT_KEY_RIGHT:
        if(P->shootMode == false){ //Walking mode
            if(P->getPlayerLoc().x + 1 < mazeSize){
                if(!(myMatrix[P->getPlayerLoc().x + 1][P->getPlayerLoc().y] == 1)){
                    checkArrows(P->getPlayerLoc().x + 1, P->getPlayerLoc().y);
                    checkChest(P->getPlayerLoc().x + 1, P->getPlayerLoc().y);

                    myMatrix[P->getPlayerLoc().x][P->getPlayerLoc().y] = 0;
                    P->movePlayer("right");                                         //Moves player right one tile
                    myMatrix[P->getPlayerLoc().x][P->getPlayerLoc().y] = 3;

                    moveEnemies();

                    cout << endl;
                    PrintMatrix();
                }
            }
        }
         else if(P->hasArrows){ //Shooting mode
            P->playerDir = "right";
            P->shootArrow();
         }
         break;

   }
  glutPostRedisplay();
}

/* Program entry point */
int main(int argc, char *argv[])
{

   glutInit(&argc, argv);

   glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
   glutInitWindowSize (1024, 768);                //window screen
   glutInitWindowPosition (230, 40);            //window position
   glutCreateWindow ("The ENDGAME");                    //program title
   init();

   glutDisplayFunc(display);                     //callback function for display
   glutReshapeFunc(resize);                   //callback for reshape
   //glutKeyboardFunc(key);                        //callback function for keyboard
   glutSpecialFunc(Specialkeys);
   glutMouseFunc(mouse);
   glutIdleFunc(idle);
   glutMainLoop();

    deleteAll();
   return EXIT_SUCCESS;
}
