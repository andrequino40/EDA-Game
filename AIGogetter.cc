#include "Player.hh"


/**
 * Write the name of your player and save this file
 * with the same name and .cc extension.
 */
#define PLAYER_NAME Gogetter


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

  const vector<Dir> dirsWizard = {Up,Right,Down,Left};
  const vector<Dir> dirsGhost = {Down, DR, Right, RU, Up, UL, Left , LD};
  vector<int> myWizards = wizards(me());

  bool pos_free(Pos pos) {
    return pos_ok(pos) and cell(pos).type != Wall;
  }

  // bfs, returns cords with closest book
  // updates for each visited cell
  Pos bfs(GridCoords& parents, Pos start){
    
    GridBool visited(board_rows(), vector<bool>(board_cols()));
    // if still -1, no treasures
    //Pos closest_book = {-1,-1};
    queue<Pos> qcoord;


    qcoord.push(start);
    visited[start.i][start.j] = true;
    parents[start.i][start.j] = {-1,-1};


    while (not qcoord.empty()) {
        Pos current = qcoord.front();
        int i = current.i;
        int j = current.j;
        qcoord.pop();
        
        // if a book, return.
        if (cell(current).book) {
           return current;
        }

        visited[i][j] = true;
        
        //vector<coords> directions = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};  // down, up, left, right
        // add every neighboring cell to the queue
        for (const Dir dir : dirsWizard) {
          Pos newPos = current + dir;
          if (pos_free(newPos)) {
            parents[newPos.i][newPos.j] = current;
            qcoord.push(newPos);
          }
        }
    }

    // no book found
    return Pos(-1,-1);
}
  // trace path, traces back to first movement

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
      
      GridCoords parents(n, (vector<coords>(m)));
    int shortest_dist = -1;

    Pos shortest_coords = bfs(parents);
    if (shortest_coords == coords(-1,-1))
        cout << "no es pot arribar a cap tresor" << endl;
    else {
        trace_path(graph, parents, shortest_coords);
        print_path(graph);
        cout << "distancia mínima: " << shortest_dist << endl;
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
