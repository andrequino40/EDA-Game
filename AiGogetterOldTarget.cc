#include "Player.hh"
#include <map>


/**
 * Write the name of your player and save this file
 * with the same name and .cc extension.
 */
#define PLAYER_NAME GogetterTarget


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
  struct TargetInfo {
    int dist;
    int unitId;
    Dir firstMove;
  };

  map<Pos, TargetInfo> targetMap;

  const int bfsMaxDist = 10;

  Pos bfsBook(GridCoords& parents, Pos start, int id){
    
    GridInt distances(board_rows(), vector<int>(board_cols(), -1));
    // if still -1, no treasures
    //Pos closest_book = {-1,-1};
    queue<Pos> qcoord;

    qcoord.push(start);
    distances[start.i][start.j] = 0;
    parents[start.i][start.j] = Pos(-1,-1);


    while (not qcoord.empty()) {
        Pos currentPos = qcoord.front();
        int currentDist = distances[currentPos.i][currentPos.j];
        qcoord.pop();
        
        // if a book, return.
        if (cell(currentPos).book) {
          // if target doesn't exit, create,
            // create action for unit, 
          // if target exist and lower distance, modify, return
            // create action for unit
            // erase old action for unit
          // else, leave
          bool hasTarget = targetMap.find(currentPos) != targetMap.end();
          if (!hasTarget || currentDist < targetMap.at(currentPos).dist) {
            TargetInfo newTarg;
            newTarg.dist = currentDist; 
            newTarg.unitId = id;
            targetMap[currentPos] = newTarg;
            return currentPos;
          } 

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

    Pos current = end;  
    Pos parent = parents[current.i][current.j];
    while (parents[parent.i][parent.j] != Pos(-1,-1)) {
      current = parent;
      parent = parents[current.i][current.j];
    }

    Pos difference = subtractPos(current, parent);
    Dir firstMove = pos_to_dir(difference);
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
    // calculate bfs move
    Dir bfs_move = Down;
    Pos closest_book;
    for (int id : ws) {
      GridCoords parents(board_rows(), (vector<Pos>(board_cols())));
      Pos posWizard = unit(id).pos; 
      Pos closest_book = bfsBook(parents, posWizard, id);

      // cerr << "Wizard bfs: "  << posWizard.i << " , " << posWizard.j << endl;
      // cerr << "> ID: "  << id << endl;

      if (closest_book == Pos(-1,-1)) {
        bfs_move = dirsWizard[random(0, dirsWizard.size() -1)];
      } else {
        bfs_move = trace_path(parents, closest_book);
      } 
    }

    for (int id : ws) {
      
    }
      //rotation code
      /* for (int id : ws) {
        Dir d = dirsWizard[whatDir];
        Pos new_pos = unit(id).pos + d;
        if (pos_free(new_pos)) move(id,d);
      }

      whatDir++;
      whatDir%=4; */
  }
};


/**
 * Do not modify the following line.
 */
RegisterPlayer(PLAYER_NAME);
