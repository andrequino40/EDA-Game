#include "Player.hh"


/**
 * Write the name of your player and save this file
 * with the same name and .cc extension.
 */
#define PLAYER_NAME Rotator


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

  const vector<Dir> dirsWizard = {Up,Right,Down,Left};
  const vector<Dir> dirsGhost = {Down, DR, Right, RU, Up, UL, Left , LD};
  vector<int> myWizards = wizards(me());

  bool pos_free(Pos pos) {
    return pos_ok(pos) and cell(pos).type != Wall;
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

      for (int id : ws) {
        Dir d = dirsWizard[whatDir];
        Pos new_pos = unit(id).pos + d;
        if (pos_free(new_pos)) move(id,d);
      }

      whatDir++;
      whatDir%=4;
  }
};


/**
 * Do not modify the following line.
 */
RegisterPlayer(PLAYER_NAME);
