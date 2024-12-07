#include "Player.hh"
#include <map>
#include <unordered_map>
#include <set>
#include <list>


/**
 * Write the name of your player and save this file
 * with the same name and .cc extension.
 */
#define PLAYER_NAME LilBro


struct PLAYER_NAME : public Player {

  /**
   * Factory: returns a new instance of this class.
   * Do not modify this function.
   */
  static Player* factory () {
    return new PLAYER_NAME;
  }

  /* AUXILIARY FUNCTIONS */
  bool show_debug_messages = false;
  
  Dir get_opposite_dir(Dir d){
    Dir res = (Dir)d;
    if (d == Down)        res = Up;   
    else if (d == DR)     res = UL;     
    else if (d == Right)  res = Left;  
    else if (d == RU)     res = LD;     
    else if (d == Up)     res = Down;     
    else if (d == UL)     res = DR;     
    else if (d == Left)   res = Right;   
    else if (d == LD)     res = RU;  
    else {
      cerr << "> err: wrong direction to get_opposite_dir" << endl;
    }   
    return res;
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
    
    bool isCorridor(Pos pos) {
    return pos_ok(pos) and cell(pos).type != Wall;
  }

  int manhattanDistance(Pos start, Pos target) {
    return abs(start.i - target.i) + abs(start.j - target.j);
  } 

  vector<Dir> getVectorDirectionsForUnitType(int id) {
    if (unit(id).type == Wizard) return dirsWizard;
    return dirsGhost; 
  }

  /* TYPES AND ATTRIBUTES */
  typedef vector<vector<Pos>> GridCoords;
  typedef vector<vector<bool>> GridBool;
  typedef vector<vector<int>> GridInt;

  vector<int> enemyGhosts;
  const vector<Dir> dirsWizard = {Up,Right,Down,Left};
  const vector<Dir> dirsGhost = {Down, DR, Right, RU, Up, UL, Left , LD};

  // ATTRIBUTES ACTIONS
  enum ActionType {
    book, fightMage, attackGhost, helpMage, runMage, runVoldemort, cast
  };

  vector<ActionType> vecAllActions = {
     book, fightMage, attackGhost, helpMage, runMage, runVoldemort, cast
  };

  // from more to least importance
  vector<ActionType> vecMageActions = {
    helpMage, fightMage, attackGhost, book, runMage, runVoldemort
  };

  // from more to least importance
  vector<ActionType> vecGhostActions = {
    runVoldemort, runMage, cast, book
  };

  struct ActionData {
    // ActionType actionType;  // The type of action
    bool cando = false;
    int distance;           // Distance (used for book, fightMage, runMage, helpMage, attackGhost)
    int firstStep;            // First movement direction (for book)
    int enemyId;            // Enemy ID (for fightMage, fightGhost)
    int numbersAdvantage;   // Numbers advantage (for fightMage, runMage, attackGhost)
    // Other fields can be added for more specific use cases
};


  typedef unordered_map<int, vector<int>> idToActions_t;
  typedef unordered_map<int, vector<ActionData>> new_idToActions_t;

  // ATTRIBUTES BOOKS

  bool search_again_replaced = true;
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

// ATTRIBUTES SPELL

  typedef vector<int> vint;
typedef const int cint;

// ATTRIBUTES ASTAR

struct node_t {
      Pos pos;
      int firstStep = -1;
      int gcost;
      int heuristic;
  };

struct aStarRes_t {
  bool found = false;
  int firstStep = -1;
  int dist;
};

// ATTRIBUTES MAGEBFS
struct bookMageBfs_t {
  bool found = false;
  Pos pos;
  int firstStep;
  int dist;
};

/* FUNCTIONS BOOKS*/

bookBfsRes_t bfsClosestToBook(const int id, const posToBookinfo_t& targetBooks){
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
        if (isCorridor(newPos)){

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
 
  int setBookAction(const int id, const bookBfsRes_t book_result, posToBookinfo_t& targetBooks, new_idToActions_t& idToActions){
    if (!book_result.found) {
      cerr << ">err Book not found for " << id << endl;
      return -1;
    }

    int oldid = -1;
    bool hasTarget = targetBooks.find(book_result.pos) != targetBooks.end();

    // if the book was already targeted
    if (hasTarget) {
      // get old id from book targets
      oldid = targetBooks[book_result.pos].unitId;
      // remove action from id
      idToActions[oldid][book].cando = false;
    }
    // update target information
    BookInfo newTarg;
    newTarg.dist = book_result.dist; 
    newTarg.unitId = id;
    targetBooks[book_result.pos] = newTarg;
    // create action
    idToActions[id][book].cando = true; 
    idToActions[id][book].firstStep = (Dir)book_result.firstStep;
    idToActions[id][book].distance = book_result.dist;

    // return id that was displaced
    return oldid;
  }  
  


  void new_updateBookActionIfClosestToBook(const int id, posToBookinfo_t& targetBooks, new_idToActions_t& idToActions) {
    // this function finds if there's a book that's closest to the provided id
    // if it does, and noone else is going for it, it sets [book].cando true for the id
    // if any id gets removed from cando, it adds it to the queue until everyone has a book to go to
    queue<int> queueids;
    queueids.push(id);

    while (!queueids.empty()) {
      int id = queueids.front();
      queueids.pop();

      bookBfsRes_t book_result = bfsClosestToBook(id, targetBooks);
      int displaced_id = -1;
      if (book_result.found) {
        displaced_id = setBookAction(id, book_result, targetBooks, idToActions);

        if (search_again_replaced && displaced_id != -1) {
          queueids.push(displaced_id);
        }
      }
    }
  }

/* FUNCTIONS SPELL */

bool spellBacktrack(int i, int sum_resto, int sum_actual, vint& sumas, vint& sol, const vint& vecnums, cint ngroups, cint target) {
    int n = vecnums.size();
    int suma_total = target * ngroups;
    
    if ( i == n) {
        return sum_actual == suma_total; // creo que esto podria ser return true
    }
    bool found = false;
    int valor = vecnums[i];

    int new_resto = sum_resto - valor;
    int new_actual = sum_actual + valor;

    for (int grupo = 0; grupo < ngroups; ++grupo) {
        // comprobar que añadir el valor no hace que se pase del target
        bool below_target = sumas[grupo] + valor <= target;
        // comprobar si la suma del resto de elementos es suficiente para llegar al target (aqui se podria mirar cuanto necesitan el resto de grupos)
        bool enough_for_target = sumas[grupo] + sum_resto >= target;
        if (below_target  && enough_for_target){
            // hace la elección (sobre escribe el anterior grupo)
            sol[i] = grupo;
            sumas[grupo] += valor;
            
            // continuamos iterando con la elección
            found = spellBacktrack(i + 1, new_resto, new_actual, sumas, sol, vecnums, ngroups, target);
            
            // si hemos encontrado solución, no necesitamos el resto
            if (found) return true;

            //deshace la eleccion
            sumas[grupo] -= valor;
        }
    }
    return found; // return false
    // }
}

void spellCast(){
    cint ghostid = ghost(me());
    vint vecnums = spell_ingredients();
    cint n = vecnums.size();
    cint ngroups = n/3;
    vint sumgroups(ngroups, 0);
    vint sol(n, -1);
    int sumatorio = 0;
    for (int i = 0; i < n; ++i) {
      sumatorio += vecnums[i];
    } 
    if (show_debug_messages)
    cerr << "suma total" << sumatorio << endl;
    const int target = sumatorio/ngroups;

    // take first value for group zero
    sol[0] = 0;
    int first_value = vecnums[0];
    sumgroups[0] += first_value;
    // update resto
    int sum_resto = sumatorio - first_value;

    bool found = spellBacktrack(1, sum_resto, first_value, sumgroups, sol, vecnums, ngroups, target);
    if (found) {
      spell(ghostid, sol);
      // cerr << "spell success" << endl;   
    }

    if (show_debug_messages) {
      if (found) {
        cerr << "spell success, sol: " << endl;
        for (int i = 0; i < n; ++i) {
          cerr << vecnums[i] << " ";
        } 
        cerr << endl;
        for (int i = 0; i < n; ++i) {
          cerr << sol[i] << " ";
        } 
        cerr << endl;
        
      } else {
        cerr << "spell failed!";
      }
    }

    

}

vector<int> get_enemy_ghosts(){
  vector<int> result;
   for (int pid = 0; pid < num_players(); ++pid) {
    string whose;
    if (me() != pid) {
      result.push_back(unit(ghost(pid)).id);
    }
  }
  return result;
}

void print_ghost_cooldowns() {
    cerr << "> ghost waiting times" << endl;
    for (int pid = 0; pid < num_players(); ++pid) {
      string whose;
      if (me() == pid) {
        whose = "> My ";
      } else {
        whose = "> Other ";
      }
      cerr << whose << pid << " ghost has to wait:" << unit(ghost(pid)).resting_rounds() << endl;
    }
  }

bool shouldCast() {
  int myghost = ghost(me());
  bool rested = unit(myghost).resting_rounds() == 0;
  bool last50_rounds = round() >= 150;
  int numcasted = 0;
  for (int ghostid : enemyGhosts) {
    numcasted += unit(ghostid).resting_rounds() != 0;
  }

  if (show_debug_messages) {
    print_ghost_cooldowns();
  } 

  if (rested && !last50_rounds) {
    // ultima ronda para castear dos veces
    if (round() < 90) {
      if (numcasted == 3) return true;

      if (round() >= 63 && numcasted >= 2)
        return true;

      else if (round() >= 70 && numcasted >= 1)
        return true;

      else if(round() >= 80)
        return true;

    }
    // ultima ronda para castear es 149
    else if (round() >= 125 && numcasted >= 2) 
        return true;

    else if (round() > 130 && numcasted >= 1) 
      return true;

    else if (round() > 140) 
      return true;
  }
  return false;
}
  

  void decideMoveGhost(const int id, const vector<ActionData>& actions, posToBookinfo_t& targetBooks, new_idToActions_t& idToActions) {
    // this should be set to true if voldemort is targetting an id, or if it's withing a certain radios
    Pos position = unit(id).pos;
    if (actions[runVoldemort].cando) {
      Pos furthestPos = bfsFurthestPosVoldemort(id);
      aStarRes_t res = astarAlgorithm(id, position, furthestPos);

      move(id, (Dir)res.firstStep);
      return;
    }
    // this should be set to true if an enemy is the closest to a certain unit
    if (actions[fightMage].cando) {
      if ((actions[fightMage].distance - 4) < actions[book].distance) {
        // bfs furthest from mage
        // return result
        return;
      }
    }

    if (actions[cast].cando) {
      spellCast();
      return;
    }
    // sees if it has any closest book
    if (actions[book].cando) {
      Dir direction = (Dir)actions[book].firstStep;
      move(id, direction);
      return;
    }
    // return random move towards an enemy colored square if possible
  }


  void decideMoveMage(const int id, const vector<ActionData>& actions, posToBookinfo_t& targetBooks, new_idToActions_t& idToActions) {
    // this should be set to true if voldemort is targetting an id, or if it's withing a certain radios

      if (actions[helpMage].cando) {
        Dir direction = (Dir)actions[helpMage].firstStep;
        move(id, direction);
        return;
      }
      if (actions[book].cando) {
        Dir direction = (Dir)actions[book].firstStep;
        move(id, direction);
        return;
      }
    // return random move towards an enemy colored square if possible
  }

  /* FUNCTIONS ASTAR */

  static bool compAstar(const node_t& a, const node_t& b) { // true means a has less priority in std priority queues
    int afcost = a.gcost + a.heuristic;
    int bfcost = b.gcost + b.heuristic;
    if (afcost > bfcost) {
        return true; // if afcost cost is greater, it has less priority
    }
    if (afcost == bfcost) { // if both have the same fcost
        return a.heuristic > b.heuristic; // if a's distance from end is greater, it has less priority
    }
    return false; // if afcost is lower, it has more priority
  } 

  typedef priority_queue<node_t, vector<node_t>, decltype(&PLAYER_NAME::compAstar)> pqastar;

  aStarRes_t astarAlgorithm(const int id, const Pos start, const Pos target){
    if (show_debug_messages){
      cerr << "astar start" << start << " target: " << target << endl;
    }
    GridBool closed (board_rows(), vector<bool>(board_cols(), false)); // false means not closed
    GridInt fscores (board_rows(), vector<int>(board_cols(), -1)); // -1 means not in queue
    pqastar pq(PLAYER_NAME::compAstar);

    vector<Dir> directions = getVectorDirectionsForUnitType(id);

    aStarRes_t result;

    node_t first;
    first.pos = start;
    first.gcost = 0;
    first.heuristic = manhattanDistance(start, target);
    pq.push(first);

    while(!pq.empty()) {
        node_t current = pq.top();
        pq.pop();   

        closed[current.pos.i][current.pos.j] = true;
        if (show_debug_messages) {
          cerr << "> astar current: " << current.pos.i << " " << current.pos.j << endl;
          cerr << "> first step: " <<  (Dir)current.firstStep << endl;
          cerr << "> astar fcost: " << current.heuristic << endl;
        }
        if (current.pos == target) {
            result.found = true;
            result.firstStep = current.firstStep;
            result.dist = current.gcost;
            return result;
        }

        for (const Dir dir : directions) {
            Pos newpos = current.pos + dir;
            bool available = isCorridor(newpos);
            if (available && closed[newpos.i][newpos.j] == false) {

              bool inqueue = (fscores[newpos.i][newpos.j] != -1);
              int newg = current.gcost + 1;
              int newh = manhattanDistance(newpos, target);
              int newf = newg + newh;
              if (!inqueue || fscores[newpos.i][newpos.j] > newf) {
                node_t newnode;
                // position of the neighbor
                newnode.pos = newpos;
                // pass on first step
                if (current.firstStep == -1) newnode.firstStep = dir;
                else newnode.firstStep = current.firstStep;
                // gcost is distance from start node
                newnode.gcost = newg;
                // heuristic is distance from target
                newnode.heuristic = newh;
                fscores[newpos.i][newpos.j] = newf;
                pq.push(newnode);
              }
            }
        }
    }

    result.found = false;
    return result;
}

  /* FUNCTIONOS VOLDEMORT*/
  
Pos bfsFurthestPosVoldemort(const int id){
    vector<Dir> directions = getVectorDirectionsForUnitType(id);
    GridBool visited(board_rows(), vector<bool>(board_cols(), false));
    queue<Pos> qcoord;

    const int CellLimit = 300;
    int furthest_dist = 0;
    int processedCells = 0;

    Pos start  = unit(id).pos; 
    Pos result = start;

    qcoord.push(start);
    visited[start.i][start.j] = true;


    while (not qcoord.empty() && processedCells < CellLimit) {
      processedCells++;

      Pos currentPos = qcoord.front();

      qcoord.pop();
      
      int currentDist = manhattanDistance(currentPos, pos_voldemort());
      // cerr <<  "> pos" << currentPos << " dist: " << currentDist << endl;
      if (currentDist > furthest_dist) {
        furthest_dist = currentDist;
        result = currentPos;

        // cerr << "! NewFurthestPos: " << result << endl;
      }

      for (const Dir dir : directions) {

        Pos newPos = currentPos + dir;

        if (isCorridor(newPos)){
          
          bool visitedCell = visited[newPos.i][newPos.j] == true;
          
          if (!visitedCell ) {
            visited[newPos.i][newPos.j] = true;
            qcoord.push(newPos);
          }
        }
      }
    }
    return result;
}
/* 
  list<Pos> voldemortTargets(const vector<Unit>& units) {
  // First collect (in possib) the positions of all units at mininum distance
    Pos volPos = pos_voldemort();
    int closest_dist = 1e8;
    list<Pos> candidates;

    for (const Unit& u : units) {
      Pos unitPos = u.pos;
      int dist = manhattanDistance(unitPos, volPos);

      if (dist < closest_dist) {
        closest_dist = dist;
        candidates = {unitPos};
      }

      else if (dist == closest_dist ) {
        candidates.push_back(unitPos);
      }
    }
  }
   */
  
  /* FUNCTIONS CONVERSION */

  struct SearchNode_t{
    Pos pos;
    int firstStep;
    int lastStep;
  };

struct SearchRet_t {
  bool found = false;
  Pos pos;
  int firstStep;
  int lastStep;
  int dist;
};

enum class SearchType {
  Book,
  AllyMage,
  HealthyMage,
  EnemyMage,
  Ghost
};  


bool isSearchType(SearchType type, Pos pos) {
  bool result = false;
  switch (type)
  {
  case SearchType::HealthyMage:{
    int idunit = (cell(pos).id);
    if (idunit == -1)  return false; // there is no unit
    
    bool myteam = unit(idunit).player == me();
    bool converting = unit(idunit).is_in_conversion_process();
    // if its from my team and its not converting, true
    return myteam && !converting;
  }
  
  default:
    break;
  }
  return result;
}

SearchRet_t bfsSearchForType(const int id, SearchType type, const int maxDist, const posToBookinfo_t& targetBooks) {
    Pos start  = unit(id).pos; 
    
    vector<Dir> directions;
    if (unit(id).type == Wizard) directions = dirsWizard;
    else directions = dirsGhost;

    SearchRet_t result;
    result.found = false;

    GridInt distances(board_rows(), vector<int>(board_cols(), -1));

    queue<SearchNode_t> qcoord;

    qcoord.push({start, -1, -1});
    distances[start.i][start.j] = 0;

    while (not qcoord.empty()) {
      Pos currentPos = qcoord.front().pos;
      int firstStep = qcoord.front().firstStep;
      int lastStep = qcoord.front().lastStep;
      int currentDist = distances[currentPos.i][currentPos.j];

      qcoord.pop();

      if (maxDist > 0 && currentDist > maxDist) continue;
      
      // if is of searched type
      if (isSearchType(type, currentPos)) {

          result.found = true;
          result.pos = currentPos;
          result.firstStep = firstStep;
          result.lastStep = lastStep;
          result.dist = currentDist;
          
          return result;
        }
      
      for (const Dir dir : directions) {
        Pos newPos = currentPos + dir;
        if (isCorridor(newPos)){

          int newDist = currentDist + 1;
          bool visited = distances[newPos.i][newPos.j] != -1;

          if (!visited) {

            distances[newPos.i][newPos.j] =  newDist;

            if (firstStep == -1) qcoord.push({newPos, dir, dir});
            else qcoord.push({newPos, firstStep, dir});
          }
        }
      }
    }
    return result;
}
 
  
  virtual void play () {


    enemyGhosts = get_enemy_ghosts();
    
    if (show_debug_messages) {
    cerr << "fantasmas ronda " << round() << endl;
    print_ghost_cooldowns();
    }

    double st = status(me());
    if (st >= 0.9) return;
  
    int my_ghost = ghost(me());
    vector<int> ws = wizards(me());

    posToBookinfo_t targetBooks;

    new_idToActions_t idToActions;

    queue<int> queueids;

    /* QUEUE OF IDS*/

    // for every wizard, initializes its vector of the size of their possible actions
    for (int wizard_id : ws) {
      idToActions[wizard_id] = vector<ActionData>(vecAllActions.size());
      // creates a queue of every wizard id
    }
    idToActions[my_ghost] = vector<ActionData>(vecAllActions.size());
    

    /* GHOST */

    // see if the ghost can cast a spell
    idToActions[my_ghost][cast].cando = shouldCast();

    bool involarea =  (manhattanDistance(unit(my_ghost).pos, pos_voldemort()) < 15);
    idToActions[my_ghost][runVoldemort].cando = involarea;
    

    /* WIZARDS */
    for (int id: ws) {
      if (unit(id).is_in_conversion_process()) {
        int rounds_left = unit(id).rounds_for_converting();

        if (show_debug_messages || true) {
          cerr << "mago con id " << id << "en posicion " << unit(id).pos 
          << "se convertira en " <<unit(id).rounds_for_converting() << " rondas" << endl;
        }

        // last alive round is when rounds_left = 1
        // both units have to meet in rounds_left -1 moves;
        // moves * 2 = maxdistance
        int max_distance = (rounds_left -1) * 2;
        SearchRet_t result;

        if (max_distance <= 0) continue;

        result = bfsSearchForType(id, 
        SearchType::HealthyMage, max_distance, targetBooks);

        if (!result.found) continue;
        int helper_id = cell(result.pos).id;
        int convert_id = id;

        idToActions[convert_id][helpMage].cando = true;
        idToActions[convert_id][helpMage].firstStep = result.firstStep;
        idToActions[helper_id][helpMage].cando = true;
        Dir helper_first = get_opposite_dir((Dir)result.lastStep);
        idToActions[helper_id][helpMage].firstStep = helper_first;

      }
    }

    /* EVERY UNIT */
    // for every unit edit its actions if there's a book to find
    for (int id : ws) {
      // if (unit(id).is_in_conversion_process()) continue;;
      new_updateBookActionIfClosestToBook(id, targetBooks, idToActions);
    }
    new_updateBookActionIfClosestToBook(my_ghost, targetBooks, idToActions);

    for (const auto& it : idToActions) {
      int id = it.first;
      const vector<ActionData>& vecActions = it.second;

      if (unit(id).type == Ghost) decideMoveGhost(id, vecActions, targetBooks, idToActions);
      else decideMoveMage(id, vecActions, targetBooks, idToActions);
    }
  }
};


/**
 * Do not modify the following line.
 */
RegisterPlayer(PLAYER_NAME);
