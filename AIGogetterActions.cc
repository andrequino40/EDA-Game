#include "Player.hh"
#include <map>
#include <unordered_map>
#include <set>


/**
 * Write the name of your player and save this file
 * with the same name and .cc extension.
 */
#define PLAYER_NAME GogetterV6


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

  vector<int> enemyGhosts;
  const vector<Dir> dirsWizard = {Up,Right,Down,Left};
  const vector<Dir> dirsGhost = {Down, DR, Right, RU, Up, UL, Left , LD};

  /* EVERYTHING RELATED TO BOOK FINDING*/

  bool search_again_replaced = false;
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
    book, fightMage, attackGhost, helpMage, runMage, runVoldemort, cast
  };

  vector<ActionType> vecAllActions = {
     book, fightMage, attackGhost, helpMage, runMage, runVoldemort, cast
  };

  // from more to least importance
  vector<ActionType> vecMageActions = {
    fightMage, attackGhost, book, helpMage, runMage, runVoldemort
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
    int enemyId;            // Enemy ID (for fightMage)
    int numbersAdvantage;   // Numbers advantage (for fightMage, runMage, attackGhost)
    // Other fields can be added for more specific use cases
};


  typedef unordered_map<int, vector<int>> idToActions_t;
  typedef unordered_map<int, vector<ActionData>> new_idToActions_t;

  
  int setBookAction(const int id, const bookBfsRes_t book_result, posToBookinfo_t& targetBooks, new_idToActions_t& idToActions){
    if (!book_result.found) {
      cerr << "Book not found for " << id << endl;
      return -1;
    }

    int oldid = -1;
    bool hasTarget = targetBooks.find(book_result.pos) != targetBooks.end();

    if (hasTarget && book_result.dist < targetBooks.at(book_result.pos).dist) {
      // remove action from old unit 
      oldid = targetBooks[book_result.pos].unitId;
      idToActions[oldid][book].cando = false;
    }

    BookInfo newTarg;
    newTarg.dist = book_result.dist; 
    newTarg.unitId = id;
    targetBooks[book_result.pos] = newTarg;
    // create action
    idToActions[id][book].cando = true; 
    idToActions[id][book].firstStep = (Dir)book_result.firstStep;
    idToActions[id][book].distance = book_result.dist;

    return oldid;
  }  

  

/* EVERYTHING RELATED TO THE SPELL */
typedef vector<int> vint;
typedef const int cint;

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

void spellCast(cint ghostid){
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

bool shouldCast(int myghost) {
  
  /*   if (show_debug_messages) {
        cerr << "not last50: " << !last50_rounds << endl; 
        cerr << "rested: " << rested << endl; 
    }
  */  
  bool rested = unit(myghost).resting_rounds() == 0;
  bool last50_rounds = round() >= 150;
  int numcasted = 0;
  for (int ghostid : enemyGhosts) {
    numcasted += unit(ghostid).resting_rounds() != 0;
  }

  if (rested && !last50_rounds) {
    if (show_debug_messages) {
    print_ghost_cooldowns();
    } 
    return true;
    /* if (numcasted == 3) return true;
    // ultima ronda para castear dos veces
    if (round() < 90) {
      if (round() >= 70 && numcasted > 2) {
        return true;
      } 
      else if (round() >= 80 && numcasted > 1) {
        return true;
      } 
      else if(round() >= 85) {
        return true;
      }
    }
    // ultima ronda para castear es 149
    else if (round() >= 130 && numcasted > 2) {
        return true;
      } 
      else if (round() > 140 && numcasted > 1) {
        return true;
      } 
      else if (round() > 155) {
        return true;
      } */
  }
  return false;
}
  virtual void play () {

    enemyGhosts = get_enemy_ghosts();
    
    cerr << endl;
    double st = status(me());
    if (st >= 0.9) return;
  
    int ghost_id = ghost(me());
    vector<int> ws = wizards(me());


    /*
      el fantasma podia hacerlo unit(ghost_id).resting_rounds() = 0
      no en las ultimas 150 rondas
     */
    // 50 199 no
    // 49 198 no

    // 1  150 no
    // 0  149 si
    // vector<int> ghostids(num_players());
    //por cada jugador
    

    posToBookinfo_t targetBooks;

    new_idToActions_t idToActions;

    queue<int> queueids;

    // for every wizard, initializes its vector of the size of their possible actions
    for (int wizard_id : ws) {
      idToActions[wizard_id] = vector<ActionData>(vecAllActions.size());
      // creates a queue of every wizard id
      queueids.push(wizard_id);
    }
    
    // for the ghost, initializes its vector with the size of his possible actions
    idToActions[ghost_id] = vector<ActionData>(vecAllActions.size());
    queueids.push(ghost_id);

    const queue<int> ref_queueids = queueids;

    // for every wizard in the queue, edit its actions if there's a book to find
    while (!queueids.empty()) {
      int id = queueids.front();
      queueids.pop();

      bookBfsRes_t book_result = findBookCandidate(id, targetBooks);


      if (book_result.found) {
        int displaced_id = setBookAction(id, book_result, targetBooks, idToActions);
        if (search_again_replaced && displaced_id != -1) {
          queueids.push(displaced_id);
        }
      }
    }


    for (const auto& it : idToActions) {
      int id = it.first;
      const vector<ActionData>& vecActions = it.second;

      vector<ActionType> possibleActions;
      if (unit(id).type == Wizard) possibleActions = vecMageActions;
      else possibleActions = vecGhostActions;
      int n = possibleActions.size();


      // iterates through all possible actions for an id
      for (int i = 0; i < n; ++i) {
        ActionType type = possibleActions[i];

        switch (type) {
        case book:
          if (!vecActions[book].cando) break;
          move(id, (Dir)vecActions[book].firstStep);
          break;

        case cast:
          if (shouldCast(ghost_id)) { 

   
            spellCast(ghost_id);
          }
        default:
          break;
        }
      }
    }
  }
};


/**
 * Do not modify the following line.
 */
RegisterPlayer(PLAYER_NAME);
