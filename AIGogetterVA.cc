#include "Player.hh"
#include <map>
#include <unordered_map>
#include <set>
#include <list>


/**
 * Write the name of your player and save this file
 * with the same name and .cc extension.
 */
#define PLAYER_NAME GogetterXIV


struct PLAYER_NAME : public Player {

  /**
   * Factory: returns a new instance of this class.
   * Do not modify this function.
   */
  static Player* factory () {
    return new PLAYER_NAME;
  }

  /* AUXILIARY FUNCTIONS */
  bool show_debug_messages = true;
  bool limit_book_converted = false;

   typedef stringstream message;

  void printcerr(bool active, const message& msg){
    if (!show_debug_messages || !active) return;
    cerr << msg.rdbuf();
  }
  template <typename T>
  void printerr(bool active, const T& msg){
    if (!show_debug_messages || !active) return;
    cerr << msg;
  }
  
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

   const vector<Dir>& getReferenceDirectonsForType(int id) {
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
    GetBook, fightMage, attackGhost, helpMage, runMage, runVoldemort, cast, color
  };

  vector<ActionType> vecAllActions = {
     GetBook, fightMage, attackGhost, helpMage, runMage, runVoldemort, cast, color
  };

  // from more to least importance
  vector<ActionType> vecMageActions = {
    helpMage, fightMage, attackGhost, GetBook, runMage, runVoldemort, color
  };

  // from more to least importance
  vector<ActionType> vecGhostActions = {
    runVoldemort, runMage, cast, GetBook, color
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

// ATTRIBUTES SEARCH

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

enum class SearchTarget {
  Book,
  AllyMage,
  HealthyMage,
  Enemy,
  Ghost,
  EnemyColor
};  

/* FUNCTIONS BOOKS*/

  int setBookAction(const int id, const SearchRet_t book_result, posToBookinfo_t& targetBooks, new_idToActions_t& idToActions){
    if (!book_result.found) {
      return -1;
    }

    int oldid = -1;
    bool hasTarget = targetBooks.find(book_result.pos) != targetBooks.end();

    // if the book was already targeted
    if (hasTarget) {
      // get old id from book targets
      oldid = targetBooks[book_result.pos].unitId;
      // remove action from id
      idToActions[oldid][GetBook].cando = false;
    }
    // update target information
    BookInfo newTarg;
    newTarg.dist = book_result.dist; 
    newTarg.unitId = id;
    targetBooks[book_result.pos] = newTarg;
    // create action
    idToActions[id][GetBook].cando = true; 
    idToActions[id][GetBook].firstStep = (Dir)book_result.firstStep;
    idToActions[id][GetBook].distance = book_result.dist;

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

      int maxdist = -1;
      // O . . . B
      // 4 3 2 1 0 (Cannot reach book)
      if (limit_book_converted) {
        if (unit(id).type == Wizard && unit(id).is_in_conversion_process()) {
          maxdist =  unit(id).rounds_for_converting() -1;
        }
      }
      SearchRet_t book_result = bfsSearchForType(id, SearchTarget::Book, maxdist, -1, targetBooks);
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



bool shouldCast() {
  int myghost = ghost(me());
  bool rested = unit(myghost).resting_rounds() == 0;
  bool last50_rounds = round() >= 150;
  int numcasted = 0;
  for (int ghostid : enemyGhosts) {
    numcasted += unit(ghostid).resting_rounds() != 0;
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
  
  int averageEnemyPower(){
    int average = 0;
    for (int pl = 0; pl < num_players(); pl++){
      average += magic_strength(pl);
    }
    average /= (num_players() -1);
    return average;
  }

  void decideMoveGhost(const int id, const vector<ActionData>& actions, posToBookinfo_t& targetBooks, new_idToActions_t& idToActions) {
    // this should be set to true if voldemort is targetting an id, or if it's withing a certain radios
    // Pos position = unit(id).pos;
    if (actions[runVoldemort].cando) {
      // Pos furthestPos = bfsFurthestPosVoldemort(id);
      // aStarRes_t res = astarAlgorithm(id, position, furthestPos);
      SearchRet_t res = newVolBfs(id);
      move(id, (Dir)res.firstStep);
      return;
    }

    if (actions[cast].cando) {
      spellCast();
      return;
    }
    // sees if it has any closest book
    
    
    
    if (actions[GetBook].cando) {
        Dir direction = (Dir)actions[GetBook].firstStep;
        move(id, direction);
        return;
    }

    SearchRet_t rescolor = bfsSearchForType(id, SearchTarget::EnemyColor, -1, -1, targetBooks);
    Dir direction = (Dir)rescolor.firstStep;
    move(id,direction);

    
    // return random move towards an enemy colored square if possible
  }

  double chance_to_win(int enemy_id){
    int enemy_pl = unit(enemy_id).player;
    double enemy_strength = magic_strength(enemy_pl);
    double my_strength = magic_strength(me());
    if (enemy_strength > (2*my_strength)) return 0;
    if (my_strength > (2*enemy_strength)) return 1;
    return  (my_strength/(enemy_strength + my_strength));
  }

  bool new_doIAttack(int enemy_id, int distance){
    // if (distance >= 6)  return false;
    
    bool is_ghost = unit(enemy_id).type == Ghost;
    bool good_chance = chance_to_win(enemy_id) > 0.5;

    bool attack = is_ghost || good_chance;
    if (attack) return true;
    // check if adjacent, attack for first attack chance
    return distance <= 2 && chance_to_win(enemy_id) > 0.4;
 
  }

  void decideMoveMage(const int id, const vector<ActionData>& actions, posToBookinfo_t& targetBooks, new_idToActions_t& idToActions) {
    // this should be set to true if voldemort is targetting an id, or if it's withing a certain radios

      if (actions[helpMage].cando) {
        Dir direction = (Dir)actions[helpMage].firstStep;
        move(id, direction);
        return;
      }

      if (actions[runVoldemort].cando) {
        SearchRet_t res = newVolBfs(id);
        move(id, (Dir)res.firstStep);
        return;
      }

      SearchRet_t rescolor = bfsSearchForType(id, SearchTarget::EnemyColor, 10, -1, targetBooks);
      idToActions[id][color].cando = rescolor.found;
      idToActions[id][color].firstStep = rescolor.firstStep;
      idToActions[id][color].distance = rescolor.dist;


      int dif_attack_book = actions[fightMage].distance - actions[GetBook].distance;
      // pos means book closer
      // neg means fight closer
      bool get_fight = dif_attack_book < 1;
      // get fight if less than 1 move further than book
      if (get_fight && actions[fightMage].cando){
          int distance = actions[fightMage].distance;
          int enemy_id = actions[fightMage].enemyId;

          bool should_attack = new_doIAttack(enemy_id, distance);

          if (should_attack) {
            Dir direction = (Dir)actions[fightMage].firstStep;
            move(id, direction);
            return;
          };
      }

      bool should_book = false;
      int avgpower = averageEnemyPower();
      double powerratio = magic_strength(me())/avgpower;
      bool underpowered = powerratio < 0.8;
      
      int dif_book_color = actions[GetBook].distance - actions[color].distance;
      // neg book closer, pos color closer
      bool get_book = dif_book_color < 4;
      // get book if less than 4 moves further than color
      if (actions[GetBook].cando) {
        
        if (underpowered || get_book){
          should_book = true;
        }
      }

      if (should_book){
        Dir direction = (Dir)actions[GetBook].firstStep;
        move(id, direction);
        return;
      }

      Dir direction = (Dir)rescolor.firstStep;
      move(id,direction);


      
      
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

    GridBool closed (board_rows(), vector<bool>(board_cols(), false)); // false means not closed
    GridInt fscores (board_rows(), vector<int>(board_cols(), -1)); // -1 means not in queue
    pqastar pq(PLAYER_NAME::compAstar);

    const vector<Dir>& directions = getReferenceDirectonsForType(id);

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
    const vector<Dir>& directions = getReferenceDirectonsForType(id);
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
      if (currentDist > furthest_dist) {
        furthest_dist = currentDist;
        result = currentPos;

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


struct volNode_t{
    Pos pos;
    int firstStep;
    Pos volPos;
  };

SearchRet_t newVolBfs(const int id){

    const int CellLimit = 300;
    int furthest_dist = 0;
    int processedCells = 0;

    Pos start  = unit(id).pos; 
    Pos volstart = pos_voldemort();

    const vector<Dir>& directions = getReferenceDirectonsForType(id);

    SearchRet_t result;
    result.found = false;

    GridBool visited(board_rows(), vector<bool>(board_cols(), false));

    queue<volNode_t> qcoord;

    qcoord.push({start, -1, volstart});
    visited[start.i][start.j] = true;

    while (not qcoord.empty() && processedCells < CellLimit) {
      processedCells++;

      Pos currentPos = qcoord.front().pos;
      int firstStep = qcoord.front().firstStep;
      Pos volPos = qcoord.front().volPos;

      qcoord.pop();
      
      int currentDist = manhattanDistance(currentPos, pos_voldemort());
      if (currentDist > furthest_dist) {
        furthest_dist = currentDist;
        
        result.found = true;
        result.pos = currentPos;
        result.firstStep = firstStep;
        // result.lastStep = lastStep;
        result.dist = currentDist;
      }

      for (const Dir dir : directions) {

        Pos newPos = currentPos + dir;
        if (isCorridor(newPos)){

          Pos newVolPos = volPos + volNextDir(volPos, newPos);
          if (inVolArea(newVolPos, newPos)) continue;
          // if next position will be dangerous, quit;

          bool visitedCell = visited[newPos.i][newPos.j] == true;
          
          if (!visitedCell ) {
            visited[newPos.i][newPos.j] = true;
            if (firstStep == -1) qcoord.push({newPos, dir, newVolPos});
            else qcoord.push({newPos, firstStep, newVolPos});
          }
        }
      }
    }
    return result;
}

  
  /* FUNCTIONS CONVERSION */
struct isSearchTarget_t{
  bool found = false;
  bool end = false;
};

bool isGhostImmune(int enemy_id){
  int last_round_attacked = unit(enemy_id).last_attack_received();
  int rounds_passed = round() - last_round_attacked;
  return rounds_passed <= rounds_no_attack_ghost();
}

bool canGhostCastThisGame(int enemy_id){
  int nextCastRound = round() + unit(enemy_id).resting_rounds();
  return nextCastRound < 150;
}
bool isSearchTarget(SearchTarget type, Pos pos, const posToBookinfo_t& targetBooks, const int dist) {
  switch (type) {

  case SearchTarget::HealthyMage:{
    int idunit = (cell(pos).id);
    if (idunit == -1)  return false; // there is no unit
    
    bool myteam = unit(idunit).player == me();
    bool converting = unit(idunit).is_in_conversion_process();
    // if its from my team and its not converting, true
    
    return myteam && !converting;
    break;
  }

  case SearchTarget::Book:{
    if (cell(pos).book) {
      bool hasTarget = targetBooks.find(pos) != targetBooks.end();
      bool iscandidate = (!hasTarget || dist < targetBooks.at(pos).dist);
      return iscandidate;
    }
    break;
  }

  case SearchTarget::EnemyColor:{
    if (cell(pos).owner != me())
    return true;
  }

  case SearchTarget::Enemy:{
    int enemy_id = cell(pos).id;
    if (enemy_id == -1) return false;
    if (unit(enemy_id).player == me()) return false;
    // [TODO] checkear cooldown del fantasma
    if (unit(enemy_id).type == Ghost) {
      bool immune = isGhostImmune(enemy_id);
      bool cancast = canGhostCastThisGame(enemy_id); 
      return !immune && cancast;
    }
    bool converting = unit(enemy_id).is_in_conversion_process();
    return !converting;
  }
  
  default:
    break;
  }
  return false;
}

bool is_enemy(const Pos position) {
  int id = cell(position).id;
  if (id == -1) return false;

  return unit(id).player != me();
}

int enemy_strength(const Pos position) {
    int id = cell(position).id;
    int player = unit(id).player;
    int strength = magic_strength(player);
    return strength;
}

int should_attack(const Pos position) {
    int enemyscore = enemy_strength(position);
    int myscore = magic_strength(me());
    if (enemyscore >= 2 * myscore) {
      return 10;
    }
    if (myscore >= 2 * enemyscore) {
      return -10;
    }
    double chance = (myscore/(myscore + enemyscore)) * 100;
    if (chance >= 60) {
      return -10;
    } 
    return 10;
}

int scanThreatsMove(const UnitType mytype, const Pos start, vector<int>& threatcount) {
  int minthreat = 10;
  threatcount = vector<int>(8, 0);
  const vector<Dir>& Directions = getReferenceDirectonsForType(mytype);

  for (auto dir : Directions) {
    Pos newpos = start + dir;
    if (isCorridor(newpos) == false) 
      continue;

    // check for enemy in exact position
    bool enemy = is_enemy(newpos);
   
    // marks position with an enemy, will not travel to this position
    if (enemy) {
      if (mytype == Ghost) {
        threatcount[dir] = 10;
      } else {
        threatcount[dir] = should_attack(newpos);
      }
      continue;
    }

    // checks for all tiles without enemies their adjacent tiles
    for (auto adjdir : dirsWizard) {
      Pos adjpos = newpos + adjdir;
      if (isCorridor(newpos) == false) 
      continue;

      if (adjpos == start) continue;                                                                           
      bool enemy = is_enemy(adjpos);
      if (enemy) {
        // add number of adjacent enemies
         if (mytype == Ghost) {

          int enemy_id = cell(adjpos).id;
          // not a threat if its a ghost
          if (unit(enemy_id).type == Wizard){
            threatcount[dir] += 1;
          }

        } else {
          int change;
          if (should_attack(newpos) > 0) change = 1;
          else change = -1;
          threatcount[dir] += change;
        }
        
      }
    }

    if (threatcount[dir] < minthreat) {
        minthreat = threatcount[dir];
    }

  }
  return minthreat;
}
SearchRet_t bfsSearchForType(const int id, SearchTarget type, const int maxDist, const int maxCells, const posToBookinfo_t& targetBooks) {
    Pos start  = unit(id).pos; 
    
    const vector<Dir>& directions = getReferenceDirectonsForType(id);

    SearchRet_t result;
    result.found = false;

    GridInt distances(board_rows(), vector<int>(board_cols(), -1));

    queue<SearchNode_t> qcoord;

    qcoord.push({start, -1, -1});
    distances[start.i][start.j] = 0;

    bool firstdir = true;
    
    vector<int> threatcount(8,0);
    const int minthreat = scanThreatsMove(unit(id).type, start, threatcount);

    int currentCells = 0;

    while (not qcoord.empty()) {
      Pos currentPos = qcoord.front().pos;
      int firstStep = qcoord.front().firstStep;
      int lastStep = qcoord.front().lastStep;
      int currentDist = distances[currentPos.i][currentPos.j];

      qcoord.pop();

      if (maxDist > 0 && currentDist > maxDist) continue;
      if (maxCells > 0 && currentCells > maxCells) continue;
      
      // if is of searched type and satisfies conditions
      if (isSearchTarget(type, currentPos, targetBooks, currentDist)) {

          result.found = true;
          result.pos = currentPos;
          result.firstStep = firstStep;
          result.lastStep = lastStep;
          result.dist = currentDist;
          
          return result;
        }
      
      currentCells++;

      for (const Dir dir : directions) {
        Pos newPos = currentPos + dir;
        if (isCorridor(newPos)){

          int newDist = currentDist + 1;
          bool visited = distances[newPos.i][newPos.j] != -1;

          if (unit(id).type == Ghost && firstdir) {
            
            if (type == SearchTarget::Book && threatcount[dir] != minthreat) {
              continue;
            }
          }

          if (!visited) {

            distances[newPos.i][newPos.j] =  newDist;

            if (firstStep == -1) qcoord.push({newPos, dir, dir});
            else qcoord.push({newPos, firstStep, dir});
          }
        }
      }
      firstdir = false;
    }
    return result;
}
 
    // int minim = 1e8;
  // list<Pos> possib;


  vector<int> myWizards;


 vector<int> getAllIds(){
  // wizards
  vector<int> all_ids;
  for (int pl = 0; pl < num_players(); pl++){
    vector<int> temp = wizards(pl);
    all_ids.insert(all_ids.end(), temp.begin(), temp.end());
    all_ids.push_back(ghost(pl));
  }
  return all_ids;
 }

  list<int> likelyVoldemortTarget(const vector<int>& allids){
    Pos pvol = pos_voldemort();
    int minim = 1e8;
    list<int> possib;
      for (const int id : allids) {
        Unit u = unit(id);
        Pos upos = u.pos;
        int dist = abs(upos.i - pvol.i) + abs(upos.j - pvol.j);
        if (dist < minim) {
          minim = dist;
          possib = {id};
        }
        else if (dist == minim) possib.push_back(id);
      }
    return possib;
  }

  bool inVolArea(const Pos pvol, const Pos position){
    int difi = abs(position.i - pvol.i);
    int difj = abs(position.j - pvol.j);
    return difi <= 1 && difj <= 1;
  }

   Dir volNextDir(const Pos vol, const Pos goal) {
    int di = goal.i - vol.i; // pos, goal abajo // neg, goal arriba
    int dj = goal.j - vol.j; // pos, goal derecha // neg, goal izq
    bool upwards = di < 0;
    bool leftwards = dj < 0;
    Dir res;
    // vertical magnitude larger
    if (abs(di) > abs(dj)) {
      if (upwards) res = Up;
      else res = Down;
    } 
    // horizontal magnitude alrger
    else if (abs(di) < abs(dj)) {
      if (leftwards) res = Left;
      else res = Right;
    } 
    // both magnitues equal
    else {
      // goal is to the left
      if (leftwards) {
        // Top Left
        if (upwards) return Up;
        // Bottom Left
        else return Down;
      } 
      // goal is to the right
      else {
        // Top right
        if (upwards) return Right;
        // bottom right
        else return Down;
      }
    }
    return res;
 }

  virtual void play () {

     double st = status(me());
    if (st >= 0.9) return;    

    enemyGhosts = get_enemy_ghosts();
  
    int my_ghost = ghost(me());
    myWizards = wizards(me());

    posToBookinfo_t targetBooks;

    new_idToActions_t idToActions;

    queue<int> queueids;

    /* QUEUE OF IDS*/

    // for every wizard, initializes its vector of the size of their possible actions
    for (int wizard_id : myWizards) {
      idToActions[wizard_id] = vector<ActionData>(vecAllActions.size());
      // creates a queue of every wizard id
    }
    idToActions[my_ghost] = vector<ActionData>(vecAllActions.size());
    

    /* GHOST */

    // see if the ghost can cast a spell

    const vector<int> allids = getAllIds();
    list<int> volTargets = likelyVoldemortTarget(allids);

    for (int id : volTargets) {
      if (unit(id).player == me()) {
        idToActions[id][runVoldemort].cando = true;
      }
    } 

    idToActions[my_ghost][cast].cando = shouldCast();
    
    for (int id : myWizards){
      if (idToActions[id][helpMage].cando) continue;  
      if (idToActions[id][runVoldemort].cando) continue; 

      bool in_vol_rea =  (manhattanDistance(unit(id).pos, pos_voldemort()) < 6);
      idToActions[id][runVoldemort].cando = in_vol_rea;
    }

    if (!idToActions[my_ghost][runVoldemort].cando) {
      bool ghost_vol_area =  (manhattanDistance(unit(my_ghost).pos, pos_voldemort()) < 10);
      idToActions[my_ghost][runVoldemort].cando = ghost_vol_area;
    }
    

    /* WIZARDS */
    for (int id: myWizards) {
      if (unit(id).is_in_conversion_process()) {
        int rounds_left = unit(id).rounds_for_converting();

        // last alive round is when rounds_left = 1
        // both units have to meet in rounds_left -1 moves;
        // moves * 2 = maxdistance
        int max_distance = (rounds_left -1) * 2;
        SearchRet_t result;

        if (max_distance <= 0) continue;

        result = bfsSearchForType(id, 
        SearchTarget::HealthyMage, max_distance, -1, targetBooks);

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
    for (int id : myWizards) {
      // skib bfs if needs help/ has to give help
      // if(idToActions[id][helpMage].cando) continue;
      // vector<int> threatcount = vector<int>(8, 0);
      // int lowestthreat = scanThreatsMove(Wizard, unit(id).pos, threatcount);

      // for (Dir dir: dirsWizard) {
      //   if (threatcount[dir] == -10) {
      //       idToActions[id][fightMage].cando = true;
      //       idToActions[id][fightMage].firstStep = dir;
      //   } 
      // }
      if (unit(id).is_in_conversion_process()) continue;
      SearchRet_t resfight = bfsSearchForType(id, SearchTarget::Enemy, -1, -1, targetBooks);
      idToActions[id][fightMage].cando = resfight.found;
      idToActions[id][fightMage].firstStep = (Dir)resfight.firstStep;
      idToActions[id][fightMage].distance = resfight.dist;
      idToActions[id][fightMage].enemyId = cell(resfight.pos).id;
    }

    for (int id : myWizards) {
      // skip bfs if needs help/ has to give help
      if(idToActions[id][helpMage].cando) continue;
      // if(idToActions[id][fightMage].cando) continue;
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

