#include "Player.hh"
#include <map>


/**
 * Write the name of your player and save this file
 * with the same name and .cc extension.
 */
#define PLAYER_NAME First


struct PLAYER_NAME : public Player {

  /**
   * Factory: returns a new instance of this class.
   * Do not modify this function.
   */
  static Player* factory () {
    return new PLAYER_NAME;
  }

  /**
   * Types and attributes for your player can be defined here.
   */
  typedef vector<vector<Pos>> GridCoords;
  typedef vector<vector<bool>> GridBool;
  typedef vector<vector<int>> GridInt;


  const vector<Dir> dirsWizard = {Up,Right,Down,Left};
  const vector<Dir> dirsGhost = {Down, DR, Right, RU, Up, UL, Left , LD};
  vector<int> myWizards = wizards(me());

  
  bool pos_free(Pos pos) {
    return pos_ok(pos) and cell(pos).type != Wall;
  }

  // bfs, returns cords with closest book
  // updates for each visited cell
  int procesadas = 0;
  struct TargetInfo {
    int dist;
    int unitId;
  };
  map<Pos, TargetInfo> TargetMap;

  const int bfsMaxDist = 30;

  Pos bfsBook(GridCoords& parents, Pos start){
    
    GridInt distances(board_rows(), vector<int>(board_cols(), -1));
    // if still -1, no treasures
    //Pos closest_book = {-1,-1};
    queue<Pos> qcoord;

    qcoord.push(start);
    distances[start.i][start.j] = 0;
    parents[start.i][start.j] = Pos(-1,-1);

    while (not qcoord.empty()) {
        procesadas++;
        Pos currentPos = qcoord.front();
        int currentDist = distances[currentPos.i][currentPos.j];
        //int i = current.i;
        //int j = current.j;
        qcoord.pop();
        
        // if a book, return.
        if (cell(currentPos).book) {
           return currentPos;
        }
        for (const Dir dir : dirsWizard) {
          Pos newPos = currentPos + dir;
          if (pos_free(newPos)){

            int newDist = currentDist + 1;
            bool visited = distances[newPos.i][newPos.j] != -1;
            if (!visited && (newDist <= bfsMaxDist)) {
              parents[newPos.i][newPos.j] = currentPos;
              distances[newPos.i][newPos.j] =  newDist;
              qcoord.push(newPos);
            } 
          }
        }
    }
    // no book found
    return Pos(-1,-1);
}

struct PosStep{
  Pos pos;
  int firstStep;
};

int bfsDirBook(Pos start){
    
    GridInt distances(board_rows(), vector<int>(board_cols(), -1));
    // if still -1, no treasures
    //Pos closest_book = {-1,-1};
    queue<PosStep> qcoord;

    qcoord.push({start, -1});
    distances[start.i][start.j] = 0;

    while (not qcoord.empty()) {
      procesadas++;
      Pos currentPos = qcoord.front().pos;
      int firstStep = qcoord.front().firstStep;
      int currentDist = distances[currentPos.i][currentPos.j];

      qcoord.pop();
      
      // if a book, return.
      if (cell(currentPos).book) {
          return firstStep;
      }
      for (const Dir dir : dirsWizard) {
        Pos newPos = currentPos + dir;
        if (pos_free(newPos)){

          int newDist = currentDist + 1;
          bool visited = distances[newPos.i][newPos.j] != -1;
          if (!visited && (newDist <= bfsMaxDist)) {

            distances[newPos.i][newPos.j] =  newDist;

            if (firstStep == -1) qcoord.push({newPos, dir});
            else qcoord.push({newPos, firstStep});
          }
        }
      }
    }
    // no book found
    return -1;
}

Dir pos_to_dir(Pos d) {
  if (d.i == 1 && d.j == 0)
    return Down;   
  if (d.i == 1 && d.j == 1)
      return DR; 
  if (d.i == 0 && d.j == 1)
      return Right;  
  if (d.i == -1 && d.j == 1)
      return RU; 
  if (d.i == -1 && d.j == 0)
      return Up;     
  if (d.i == -1 && d.j == -1)
      return UL; 
  if (d.i == 0 && d.j == -1)
      return Left;   
  if (d.i == 1 && d.j == -1)
      return LD; 
  throw invalid_argument("Invalid Pos for pos_to_dir().");
}

Pos subtractPos(Pos b, Pos a) {
  int hor = b.i - a.i; 
  int ver = b.j - a.j; 
  
  return Pos(hor, ver);
}
  // trace path, traces back to first movement
  Dir trace_path(const GridCoords& parents, Pos end){
    //cerr << "Tracing path from " << end.i << " , " << end.j << endl;
    Pos current = end;  // [W] []p [B]c
    Pos parent = parents[current.i][current.j];
    //cerr << "> parent" << parent.i << " , " << parent.j << endl;
    while (parents[parent.i][parent.j] != Pos(-1,-1)) {
      current = parent;
      parent = parents[current.i][current.j];
      //cerr << "> parent" << parent.i << " , " << parent.j << endl;
    }
    // parent is -1, -1, i need to do it after
    Pos difference = subtractPos(current, parent);
    //cerr << "> 1st move" << difference.i << " , " << difference.j << endl;
    Dir firstMove = pos_to_dir(difference);
    //cerr << "> 1st move" << firstMove << endl;
    return firstMove;

  }

  int whatDir = 0;
  /**
   * Play method, invoked once per each round.
   */

  virtual void play () {
    
    /* for (int idWizard : myWizards) {
      Dir moveDir = dirsWizard[whatDir];
      Pos newPos = unit(idWizard).pos + moveDir;
      if (pos_free(newPos))
        move(idWizard, moveDir);
    } */

    double st = status(me());
    if (st >= 0.9) return;

    // If more than halfway through, do nothing.
    //if (round() > num_rounds()/2) return;

    // Getting the wizards
    vector<int> ws = wizards(me());
    //int shortest_dist = -1;
    /* for (int id : ws) {
      GridCoords parents(board_rows(), (vector<Pos>(board_cols())));
      Pos posWizard = unit(id).pos; 
      Pos closest_book = bfsBook(parents, posWizard);

      //cerr << "Pos wizard: "  << posWizard.i << " , " << posWizard.j << endl;
      //cerr << "> ID: "  << id << endl;

      Dir direction;
      if (closest_book == Pos(-1,-1)) {
        direction = dirsWizard[random(0, dirsWizard.size() -1)];
      } else {
        direction = trace_path(parents, closest_book);
      } 
      move(id, direction);
      
    } */

   for (int id : ws) {
      GridCoords parents(board_rows(), (vector<Pos>(board_cols())));
      Pos posWizard = unit(id).pos; 
      //Pos closest_book = bfsBook(parents, posWizard);

      //cerr << "Pos wizard: "  << posWizard.i << " , " << posWizard.j << endl;
      //cerr << "> ID: "  << id << endl;

      int direction = bfsDirBook(posWizard);
      if (direction != -1){
        move(id, (Dir)direction);
      }
    }

    cerr << "BFS ha procesado " << procesadas << " celdas"<< endl;
  }
};


/**
 * Do not modify the following line.
 */
RegisterPlayer(PLAYER_NAME);
