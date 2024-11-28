#include "Player.hh"
#include <map>
#include <unordered_map>
#include <set>


/**
 * Write the name of your player and save this file
 * with the same name and .cc extension.
 */
#define PLAYER_NAME GogetterV0


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

  struct BookInfo {
    int dist;
    int unitId;
  };

  const int bfsMaxDist = 15;

struct PosStep{
  Pos pos;
  int firstStep;
};

int bfsDirBook(Pos start, map<Pos, BookInfo>& TargetBooks){
    TargetBooks.clear();
    //temp
    GridInt distances(board_rows(), vector<int>(board_cols(), -1));

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
  int numActions = 6;
  enum actionType {
    book, attackMage, attackGhost, helpMage, runMage, runVoldemort
  };

  struct actionInfo {
    actionType what;
    Dir dir;
  };

  unordered_map<actionType, Dir> typedef mapAction;

  virtual void play () {

    double st = status(me());
    if (st >= 0.9) return;
  
    vector<int> ws = wizards(me());

    map<Pos, BookInfo> TargetBooks;
    
    unordered_map<int, mapAction> idToActions;

   for (int id : ws) {
      GridCoords parents(board_rows(), (vector<Pos>(board_cols())));
      Pos posWizard = unit(id).pos; 

      int direction = bfsDirBook(posWizard, TargetBooks);
      if (direction != -1){
        idToActions[id][book] = (Dir)direction;
      }
    }

    for (const auto& it : idToActions) {
      int id = it.first;
      const mapAction& actions = it.second;
        move(id, actions.at(book));
    }

  }
};


/**
 * Do not modify the following line.
 */
RegisterPlayer(PLAYER_NAME);
