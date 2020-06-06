
#define COL_dataPin 2
#define COL_latchPin 3
#define COL_clockPin 4
#define ROW_dataPin 5
#define ROW_latchPin 6
#define ROW_clockPin 7
#define NUM_dataPin 8
#define NUM_latchPin 9
#define NUM_clockPin 10
#define SEG_dataPin 11
#define SEG_latchPin 12
#define SEG_clockPin 13

#define LEFT 0
#define UP 1
#define RIGHT 2
#define DOWN 3
#define NUMB {63 ,6 ,91, 79, 102, 109, 125, 7, 127, 111} //kolejne cyfry 7seg (wdlg Wikipedii)

// Zmienne globalne
bool Screen[8][8];  //Obraz wyswietlany na matrycy. Gdy true to odpowiadajaca dioda zostanie zapalona
char SnakeMap[8][8]; //Mapa do realizacji logiki gry
int SnakeSegments[2][64]; //Tablica przechowywujaca czlony weza
int segNumbers[10] = NUMB;
int head_x; 
int head_y;
int points;
int _move;
int timer;
int gameSpeed;
bool OFF_Control;
bool buzzer = false;
bool foodOnMap = false;
int j1,j2;

/**
 * @return dwa do potegi argumentu
 */
int pow2(int potega){
  int x=1;
  while(potega > 0){
    x*=2;
    potega--;
  }
  return x;
}

/** 
 *  @brief Wyswietlanie obrazow na matrycy LED
 */ 
void displayMatrix(bool screen[8][8]){
  for(int i = 0; i<8; i++){
    for(int j = 0; j<8; j++){
        int row = 255 - pow2(i);
        int col = pow2(j) ;
      if(screen[i][j]){
        digitalWrite(COL_latchPin, LOW);
        digitalWrite(ROW_latchPin, LOW);
        shiftOut(COL_dataPin, COL_clockPin, LSBFIRST, row);
        shiftOut(ROW_dataPin, ROW_clockPin, LSBFIRST, col);
        digitalWrite(COL_latchPin, HIGH);
        digitalWrite(ROW_latchPin, HIGH);
        digitalWrite(ROW_latchPin, LOW);
        shiftOut(ROW_dataPin, ROW_clockPin, LSBFIRST, 0); //reset dla braku artefaktow
        digitalWrite(ROW_latchPin, HIGH);
     }
     else delayMicroseconds(2); //Ok. tyle trwa proces rysowania jednego piksela.
    }   
  }
}



void displaySeg(int number){
  int num1 = number%10;
  number /=10;
  int num2 = number%10;
  digitalWrite(NUM_latchPin, LOW);
  digitalWrite(SEG_latchPin, LOW);
  shiftOut(NUM_dataPin, NUM_clockPin, LSBFIRST, 2);
  shiftOut(SEG_dataPin, SEG_clockPin, LSBFIRST, segNumbers[num1]);
  digitalWrite(NUM_latchPin, HIGH);
  digitalWrite(SEG_latchPin, HIGH);
  delayMicroseconds(1);   
  digitalWrite(NUM_latchPin, LOW);
  digitalWrite(SEG_latchPin, LOW);
  shiftOut(NUM_dataPin, NUM_clockPin, LSBFIRST, 1);
  shiftOut(SEG_dataPin, SEG_clockPin, LSBFIRST, segNumbers[num2]);
  digitalWrite(NUM_latchPin, HIGH);
  digitalWrite(SEG_latchPin, HIGH);
}

/**
 *  @brief Rzutuje SnakeMap na Screen
 */
void PrintScreen(){
  for(int x = 0; x<8; x++){
    for(int y = 0; y<8; y++){
      if(SnakeMap[x][y] != ' ') Screen[x][y] = true;
      else Screen[x][y] = false;
    }
  }
}

/**
 *  @brief Czysci Screen
 */
void ClearScreen(){
  for(int x = 0; x<8; x++){
    for(int y = 0; y<8; y++){
      Screen[x][y] = false;
     }
  }
}


/**
 * @brief Inicjalizacja nowej gry
 */
void NewGame(){
  for(int x = 0; x<8; x++){
    for(int y = 0; y<8; y++){
      if(x == y||x == 7-y) Screen[x][y] = true;
      else Screen[x][y] = false;
    }
  }
  for(int t = 0; t< 500; t++) {
    if(t%100 == 0)
      { 
        buzzer = !buzzer;
        digitalWrite(A2, buzzer);
      }
    displaySeg(0);
    displayMatrix(Screen);
  }
  buzzer = false;
  digitalWrite(A2,buzzer);
  ClearScreen(); displayMatrix(Screen);
  head_x = 4;
  head_y = 4;
  points = 0;
  _move = RIGHT;
  timer=0;
  OFF_Control = false;
  foodOnMap = false;
  for(int x = 0; x<8; x++){
    for(int y = 0; y<8; y++){SnakeMap[x][y] = ' ';}}
}

/**
 *  @brief Odczyt ruchu z przyciskow
 */
bool Control(){
  if  (!digitalRead(A0)){ _move=(_move+1)%4; return true;}  // +1 +4
  if  (!digitalRead(A1)){ _move=(_move+3)%4; return true;}  // -1 +4
  return false; 
}

/**
 *  @brief Funckja realizujaca jeden takt gry
 */
void GameStep(){
  SnakeMap[SnakeSegments[0][0]][SnakeSegments[1][0]]=' ';
  switch (_move) {
    case LEFT : head_y--; break;
    case RIGHT : head_y++; break;
    case UP : head_x--; break;
    case DOWN : head_x++; break;
  }
  if(SnakeMap[head_x][head_y]=='O'||head_x==7||head_y==0||head_x==0||head_y==7) NewGame();
  if(SnakeMap[head_x][head_y]=='X'){foodOnMap = false; points++;}
  SnakeMap[head_x][head_y]='O';
  SnakeSegments[0][points]=head_x;
  SnakeSegments[1][points]=head_y;
  for(int p = points;p>0;p--){
    SnakeSegments[0][points-p]=SnakeSegments[0][points-p+1];
    SnakeSegments[1][points-p]=SnakeSegments[1][points-p+1];
  }
  timer++; if(timer == 5) timer = 0;
  if( !timer && !foodOnMap ){
    do{
        j1=1+rand()%(6);
        j2=1+rand()%(6);
    }while(SnakeMap[j1][j2]=='O');
    SnakeMap[j1][j2]='X';
    foodOnMap = true;
    }
}

//======================
//==== ARDUINO =========
//======================

void setup() {
  for(int i = 2; i <14; i++) pinMode(i, OUTPUT); //Rejestry przesowne do sterowania matryca LED + wys 7seg x 4
  pinMode(A0, INPUT_PULLUP); //Lewy przycisk do sterowania
  pinMode(A1, INPUT_PULLUP); //Prawy przycisk do sterowania
  pinMode(A2, OUTPUT); //Buzzer
  NewGame();
}

void loop() {
  GameStep();
  OFF_Control = false;
  gameSpeed = 500 - points*20; 
  for(int t = 0; t< gameSpeed; t++){ 
    if(!OFF_Control){ if(Control()) OFF_Control = true;}
    PrintScreen();
    displayMatrix(Screen);
    displaySeg(points);
  }
}
