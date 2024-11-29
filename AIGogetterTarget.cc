#include "Player.hh"
#include <map>
#include <unordered_map>
#include <set>


/**
 * Write the name of your player and save this file
 * with the same name and .cc extension.
 */
#define PLAYER_NAME GogetterV1


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

  enum ActionType {
    book, attackMage, attackGhost, helpMage, runMage, runVoldemort
  };

  vector<ActionType> vecActions = {
    book, attackMage, attackGhost, helpMage, runMage, runVoldemort
  };

int bfsBook(Pos start, const int id, map<Pos, BookInfo>& targetBooks, unordered_map<int, vector<int>>& idToActions){
    //targetBooks.clear();
    //temp
    int displacedId = -1;

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
          bool hasTarget = targetBooks.find(currentPos) != targetBooks.end();
           if (!hasTarget) {
            // update target
            BookInfo newTarg;
            newTarg.dist = currentDist; 
            newTarg.unitId = id;
            targetBooks[currentPos] = newTarg;
            // create action
            idToActions[id][book] = (Dir)firstStep;
            return -1;
          } else if (currentDist < targetBooks.at(currentPos).dist) {
           
            // remove action from old unit 
            int oldid = targetBooks[currentPos].unitId;
            idToActions[oldid][book] = -1;
             // update target
            BookInfo newTarg;
            newTarg.dist = currentDist; 
            newTarg.unitId = id;
            targetBooks[currentPos] = newTarg;
            // create action
            idToActions[id][book] = (Dir)firstStep;
            // return 
            return oldid;
          }
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
    return displacedId;
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













  virtual void play () {

    double st = status(me());
    if (st >= 0.9) return;
  
    vector<int> ws = wizards(me());

    map<Pos, BookInfo> TargetBooks;

    unordered_map<int, vector<int>> idToActions;

    queue<int> queueids;
    for (int id : ws) {
      idToActions[id] = vector<int>(vecActions.size(), -1);
      queueids.push(id);
    }

   while (!queueids.empty()) {
      int id = queueids.front();
      queueids.pop();

      Pos posWizard = unit(id).pos; 

      int displacedid = bfsBook(posWizard, id, TargetBooks, idToActions);
      if (displacedid != -1) {
        queueids.push(displacedid);
      }
    }

    for (const auto& it : idToActions) {
      int id = it.first;
      const vector<int>& actions = it.second;

      if (actions[book] != -1) {
        move(id, (Dir)actions[book]);
      }
    }
  }
};


/**
 * Do not modify the following line.
 */
RegisterPlayer(PLAYER_NAME);
