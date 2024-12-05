#include "Player.hh"
#include <map>
#include <unordered_map>
#include <set>


/**
 * Write the name of your player and save this file
 * with the same name and .cc extension.
 */
#define PLAYER_NAME GogetterV4


struct PLAYER_NAME : public Player {

  /**
   * Factory: returns a new instance of this class.
   * Do not modify this function.
   */
  static Player* factory () {
    return new PLAYER_NAME;
  }

  

  /* AUXILIARY FUNCTIONS */

  
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
    
    bool pos_free(Pos pos) {
    return pos_ok(pos) and cell(pos).type != Wall;
  }

  /* TYPES AND ATTRIBUTES */
  typedef vector<vector<Pos>> GridCoords;
  typedef vector<vector<bool>> GridBool;
  typedef vector<vector<int>> GridInt;


  const vector<Dir> dirsWizard = {Up,Right,Down,Left};
  const vector<Dir> dirsGhost = {Down, DR, Right, RU, Up, UL, Left , LD};

  /* EVERYTHING RELATED TO BOOK FINDING*/

  bool seach_again_replaced = false;
  struct BookInfo {
    int dist;
    int unitId;
  };

  const int bfsMaxDist = 15;
  const int bfsMaxCells = 481;

  struct BookNode{
    Pos pos;
    int firstStep;
  };

struct bookBfsRes_t {
  bool found = false;
  Pos pos;
  int firstStep;
  int dist;
};

  typedef map<Pos, BookInfo> posToBookinfo_t;

bookBfsRes_t findBookCandidate(const int id, const posToBookinfo_t& targetBooks){
    Pos start  = unit(id).pos; 
    vector<Dir> directions;
    if (unit(id).type == Wizard) directions = dirsWizard;
    else directions = dirsGhost;

    bookBfsRes_t result;
    result.found = false;

    int processedCells = 0;

    GridInt distances(board_rows(), vector<int>(board_cols(), -1));

    queue<BookNode> qcoord;

    qcoord.push({start, -1});
    distances[start.i][start.j] = 0;

    while (not qcoord.empty()) {
      processedCells++;
      Pos currentPos = qcoord.front().pos;
      int firstStep = qcoord.front().firstStep;
      int currentDist = distances[currentPos.i][currentPos.j];

      qcoord.pop();
      
      // if a book and no target, or closer to it, return.
      if (cell(currentPos).book) {
        bool hasTarget = targetBooks.find(currentPos) != targetBooks.end();
        if (!hasTarget || currentDist < targetBooks.at(currentPos).dist) {

          result.found = true;
          result.pos = currentPos;
          result.firstStep = firstStep;
          result.dist = currentDist;
          
          return result;
        }
      }
      
      for (const Dir dir : directions) {
        Pos newPos = currentPos + dir;
        if (pos_free(newPos)){

          int newDist = currentDist + 1;
          bool visited = distances[newPos.i][newPos.j] != -1;

          if (!visited) {

            distances[newPos.i][newPos.j] =  newDist;

            if (firstStep == -1) qcoord.push({newPos, dir});
            else qcoord.push({newPos, firstStep});
          }
        }
      }
    }
    return result;
}
 
/* EVERYTHING RELATED TO ACTIONS */
  enum ActionType {
    book, attackMage, attackGhost, helpMage, runMage, runVoldemort
  };

  vector<ActionType> vecMageActions = {
    book, attackMage, attackGhost, helpMage, runMage, runVoldemort
  };

  vector<ActionType> vecGhostActions = {
    book, runMage, runVoldemort
  };

  typedef unordered_map<int, vector<int>> idToActions_t;


  int setBookAction(const int id, const bookBfsRes_t book_result, posToBookinfo_t& targetBooks, idToActions_t& idToActions){
    if (!book_result.found) {
      cerr << "Book not found for " << id << endl;
      return -1;
    }

    int oldid = -1;
    bool hasTarget = targetBooks.find(book_result.pos) != targetBooks.end();

    if (hasTarget && book_result.dist < targetBooks.at(book_result.pos).dist) {
      // remove action from old unit 
      oldid = targetBooks[book_result.pos].unitId;
      idToActions[oldid][book] = -1;
    }

    BookInfo newTarg;
    newTarg.dist = book_result.dist; 
    newTarg.unitId = id;
    targetBooks[book_result.pos] = newTarg;
    // create action
    idToActions[id][book] = (Dir)book_result.firstStep;

    return oldid;
  }  

  virtual void play () {
    cerr << "ingredients: ";
  vector<int> ingredients = spell_ingredients();
  int n = ingredients.size();
    for (int i = 0; i < n; ++i) {
      cerr << ingredients[i] << " ";
    } 
    cerr << endl;
    double st = status(me());
    if (st >= 0.9) return;
  
    int ghost_id = ghost(me());
    vector<int> ws = wizards(me());

    posToBookinfo_t targetBooks;

    idToActions_t idToActions;

    queue<int> queueids;

    // for every wizard, initializes its vector of the size of their possible actions
    for (int wizard_ud : ws) {
      idToActions[wizard_ud] = vector<int>(vecMageActions.size(), -1);
      // creates a queue of every wizard id
      queueids.push(wizard_ud);
    }
    
    // for the ghost, initializes its vector with the size of his possible actions
    idToActions[ghost_id] = vector<int>(vecGhostActions.size(), -1);
    queueids.push(ghost_id);

    const queue<int> ref_queueids = queueids;

    // for every wizard in the queue, edit its actions if there's a book to find
    while (!queueids.empty()) {
      int id = queueids.front();
      queueids.pop();

      bookBfsRes_t book_result = findBookCandidate(id, targetBooks);


      if (book_result.found) {
        int displaced_id = setBookAction(id, book_result, targetBooks, idToActions);
        if (seach_again_replaced && displaced_id != -1) {
          queueids.push(displaced_id);
        }
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
